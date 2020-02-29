#include "pch.h"
#include "MultiCXPSource.h"

#ifdef _DEBUG
#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

MemoryManager::MemoryManager()
{
	m_data.clear();
	m_buffers.clear();
	m_currentBuffer = 0;
	m_bufferSize = 0;
	m_isStoring = false;
}

MemoryManager::~MemoryManager()
{
	if (m_buffers.size() > 0)
	{
		for (void* b : m_buffers)
		{
			VirtualFree(b, 0, MEM_RELEASE);
		}
		m_buffers.clear();
		m_data.clear();
	}
}

size_t MemoryManager::Init(float usage, size_t buffersize)
{
	// todo if already init free old buffers

	m_bufferSize = buffersize;

	// find how much physical memory is available 
	MEMORYSTATUSEX status;
	status.dwLength = sizeof(status);
	GlobalMemoryStatusEx(&status);

	// find how many buffer can be allocated 
	if (usage > 0.9)
		usage = (float)0.9;
	if (usage < 0.05)
		usage = (float)0.05;

	size_t size = (size_t)(ceil((double)buffersize / 1024)) * 1024;
	size = (size < 1024) ? 1024 : size;
	double fcount = floor(((double)status.ullAvailPhys * usage) / 1024) / ((double)size / 1024);
	size_t count = (size_t)floor(fcount);

	// allocate all buffers
	for (int i = 0; i < count; i++)
	{

		void* data = nullptr;
		data = (void*)VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

		if (data == nullptr)
		{
			// this should not happened ... prematurly run out of memory
			// keep what ever we could get
			break;
		}
		m_buffers.push_back(data);
	}
	return m_buffers.size();
}

bool MemoryManager::SetBuffer(void* buffer, uint64_t time)
{
	// not enought buffers
	if (m_buffers.size() < 1)
		return false;
	// all buffers used
	if (m_buffers.size() <= m_data.size())
		return false;

	// find free spot in the data vector
	void* buf = m_buffers[m_data.size()];
	// copy the data
	memcpy(buf, buffer, m_bufferSize);

	// store pointer and time stamps
	m_data.push_back(std::make_pair(time, buf));
	
	return true;
}

bool MemoryManager::GetBuffer(void** buffer, int64_t increment_us, uint64_t& time)
{
	if (m_isStoring)
		return false;	// still storing
	if (m_data.size() == 0)
		return false;	// no data

	uint64_t ft64;
	// are we out of boundary
	if (m_currentBuffer >= m_data.size())
	{
		time = (*m_data.end()).first;
		m_currentBuffer = (int)m_data.size();
		return true;
	}

	// find current sample
	ft64 = increment_us;
	/*ft64 = m_data[m_currentBuffer].first;
	ft64 += (increment_us * 10);
	if (ft64 > 0x8000000000000000)
		return false;*/

	void* ptr = nullptr;
	// search the right item in the list
	std::vector<dpair>::iterator it = std::lower_bound(m_data.begin(), m_data.end(), std::make_pair(ft64, ptr), 
		[](dpair test, dpair val) -> bool { return (test.first < val.first); });
	
	m_currentBuffer = (int)(it - m_data.begin());	// keep current index in case you want to pause and then step in ( using getNext buffer)

	// retrieve the buffer
	*buffer = m_data.at(m_currentBuffer).second;
	// retrieve the time
	time = m_data.at(m_currentBuffer).first;
	return true;
}

bool MemoryManager::SeekBuffer(void** buffer, int64_t frombegin_us, uint64_t& time)
{
	if (m_isStoring)
		return false;	// still storing
	if (m_data.size() == 0)
		return false;	// no data

	uint64_t ft64;

	// find first sample
	ft64 = frombegin_us;
	/*ft64 = m_data[0].first;
	ft64 += (frombegin_us * 10);
	if (ft64 > 0x8000000000000000)
		return false;*/

	void* ptr = nullptr;
	// search the right item in the list
	std::vector<std::pair<uint64_t, void*>>::iterator it = std::lower_bound(m_data.begin(), m_data.end(), std::make_pair(ft64,ptr),
		[](dpair test, dpair val) -> bool { return (test.first < val.first); });

	m_currentBuffer = (int)(it - m_data.begin());	// keep current index in case you want to pause and then step in ( using getNext buffer)

	// retrieve the buffer
	*buffer = m_data.at(m_currentBuffer).second;
	// retrieve the time
	time = m_data.at(m_currentBuffer).first;
	return true;
}

bool MemoryManager::GetNextBuffer(void** buffer, uint64_t& time)
{
	if (m_isStoring)
		return false;	// still storing
	if (m_data.size() == 0)
		return false;	// no data
	if (m_currentBuffer >= m_data.size())
		return false;	// no more buffers

	*buffer = (m_data.at(m_currentBuffer)).second;
	time = (m_data.at(m_currentBuffer)).first;
	m_currentBuffer++;
	return true;
}

bool MemoryManager::GetNextBufferEx(void** buffer, int skip, uint64_t& time)
{
	if (m_isStoring)
		return false;	// still storing
	if (m_data.size() == 0)
		return false;	// no data
	if ((size_t)m_currentBuffer + skip >= m_data.size())
		return false;	// no more buffers
	if ((size_t)m_currentBuffer + skip < 0)
		return false;
	*buffer = (m_data.at((size_t)m_currentBuffer + skip)).second;
	time = (m_data.at((size_t)m_currentBuffer + skip)).first;
	m_currentBuffer+=skip;
	return true;
}


bool MemoryManager::StartStoring()
{
	if (m_isStoring)
		return false;
	else
	{
		m_isStoring = true;
		if (m_data.size() > 0)
		{
			// we have already data ... delete them
			m_data.clear();
			m_currentBuffer = 0;
		}

	}
	return true;
}

bool MemoryManager::StopStoring()
{
	if (!m_isStoring)
		return false;

	// make sure we can access the data
	m_isStoring = false;
	m_currentBuffer = 0;
	if (m_data.size() > 1)
	{
		// make sure everything is sorted so we can search the data 
		std::sort(m_data.begin(), m_data.end());
	}
	return true;
}

bool MemoryManager::GetRange(uint64_t& ms, uint64_t& begin, uint64_t& end)
{
	if (m_isStoring)
		return false;
	size_t s = m_data.size();
	if ( 0 == s)
		return false;

	// get begin
	begin = m_data.at(0).first;

	// get end
	end = m_data.at(s-1).first;

	// get range
	uint64_t range = end - begin;
	ms = range / 1000;

	return true;
}

size_t MemoryManager::IsStoring()
{
	if (!m_isStoring)
		return 0;
	return m_data.size();
}

//////////////////////////////////////////////////
//////////////////////////////////////////////////
void acqThread(MultiCXPSource* source)
{
	size_t s = source->m_sizeX * source->m_sizeY *  3 ;
	std::vector<std::pair<Buffer, codT*>> bufnfo;
	while (source->m_brun)
	{

		try
		{
			// get buffers from output of each grabber (pop out of output queue)
			for (uint32_t i = 0; i < source->m_lnkCnt; i++)
			{
				codT* g = source->m_grabberlist[i];
				Buffer b = (*g).pop((uint64_t)5000000);
				std::pair<Buffer, codT*> p(b,g);
				bufnfo.push_back(p);
			}

			// get buffer this first grabber hold alway ( for the S990 and S640) the pointer to the first pixel
			uint8_t* imagePointer = ((bufnfo[0]).first).getInfo<uint8_t*>(*(bufnfo[0].second),GenTL::BUFFER_INFO_BASE);
			uint64_t buftime = ((bufnfo[0]).first).getInfo<uint64_t>(*(bufnfo[0].second),GenTL::BUFFER_INFO_TIMESTAMP);
			// preview image needed
			if (source->m_copybuf)
			{
				// convert the image to the right format

				FormatConverter::BGR8 bgr(*(source->m_pconverter), imagePointer, 
					(bufnfo[0].first).getInfo<uint64_t>(*(bufnfo[0].second),GenTL::BUFFER_INFO_PIXELFORMAT),
					source->m_sizeX,
					source->m_sizeY);

				memcpy(source->m_buff, bgr.getBuffer(), s);
				source->m_copybuf = false;
			}
			if (source->m_bRec)
			{
				if (!source->m_mm->SetBuffer((void*)imagePointer, buftime))
				{
					// we could not store the buffer ... stop
					source->m_mm->StopStoring();
					source->m_bRec = false;
				}
			}

			// return each buffer to its own grabber ( push to input queue)
			for (auto nfo : bufnfo)
			{
				(nfo.first).push(*(nfo.second));
			}
			bufnfo.clear();
		}
		catch (...)
		{

		}
	}
}



int MultiCXPSource::buildGrabbers()
{
	try
	{
		m_pgentl = new EGenTL();
		m_pconverter = new FormatConverter(*m_pgentl);
	}
	catch (...)
	{
		// access the driver failed can do nothing 
		return ERROR_NODRIVER;
	}
	m_pgentl->memento(std::string("Source open driver"));

	// find how many device we have
	EGrabber<CallbackOnDemand>* grab;

	try
	{
		gc::TL_HANDLE tlh = m_pgentl->tlOpen();
		int num = m_pgentl->tlGetNumInterfaces(tlh);					// find interfaces num

		// per interface
		for (int iid = 0; iid < num; iid++)								// get all interface
		{
			std::string ifid = m_pgentl->tlGetInterfaceID(tlh, iid);
			gc::IF_HANDLE ifh = m_pgentl->tlOpenInterface(tlh, ifid);

			int count = checkHardware(ifh);

			if ( count & 0x1)
			{
				m_cameraList.push_back(std::make_pair(iid, 0));
			}
			if ( count & 0x2)
			{
				m_cameraList.push_back(std::make_pair(iid, 1));
			}

			m_pgentl->ifClose(ifh);										// free Interfaces

		}
		m_pgentl->tlClose(tlh);											// close all handles  as EGrabber need them
		m_pgentl->memento(std::string("GetGENTL: retrieved board/camera count"));

		//try to create the devices
		for (std::pair<int,int> cam : m_cameraList)
		{
			std::ostringstream os;
			os << "----- Create Grabber " << (cam.first + 1) << "/" << m_cameraList.size() << " , " << (cam.second + 1);
			m_pgentl->memento(os.str());
			try
			{
				grab = new EGrabber<CallbackOnDemand>(*m_pgentl, cam.first, cam.second);
				m_grabberlist.push_back(grab);  // save devices
			}
			catch (...)
			{
				std::ostringstream os1;
				os1 << "----- Create Grabber filed for " << (cam.first + 1) << "/" << m_cameraList.size() << " , " << (cam.second + 1) << " failed";
				m_pgentl->memento(os1.str());

			}
			
		}
	}
	catch (...)
	{
		// access to device or interface failed 
		return ERROR_ACCESSDEVICE;
	}
	return SUCCESS;
}

int MultiCXPSource::configS990(size_t pitch, size_t payload)
{
	size_t stripeHeight = 2; // 8 / m_lnkCnt; // for all configuration
	size_t stripePitch = stripeHeight * m_lnkCnt;

	try
	{
		for (size_t ix = 0; ix < m_lnkCnt; ++ix)
		{
			//S990 configure camera

			/*m_grabberlist[ix]->setInteger<RemoteModule>("Width", static_cast<int64_t>(width));
			m_grabberlist[ix]->setInteger<RemoteModule>("Height", static_cast<int64_t>(height));
			m_grabberlist[ix]->setString<RemoteModule>("PixelFormat", pixelFormat);*/
			// configure stripes on grabber data stream
			m_grabberlist[ix]->setInteger<StreamModule>(std::string("LinePitch"), pitch);
			m_grabberlist[ix]->setInteger<StreamModule>(std::string("LineWidth"), pitch);
			m_grabberlist[ix]->setInteger<StreamModule>(std::string("StripeHeight"), stripeHeight);
			m_grabberlist[ix]->setInteger<StreamModule>(std::string("StripePitch"), stripePitch);
		}
		for (uint32_t i = 0; i < m_bufferCount; i++)
		{
			m_basebuf[i] = static_cast<uint8_t*>(malloc(payload));
			for (size_t ix = 0; ix < m_lnkCnt; ++ix)
			{
				size_t offset = pitch * stripeHeight * ix;
				//S990
				m_grabberlist[ix]->announceAndQueue(UserMemory(m_basebuf[i] + offset, payload - offset));
			}
		}
	}
	catch (...)
	{
		return ERROR_PARAMACCESS;
	}
	return SUCCESS;
}

int MultiCXPSource::configS640(size_t pitch, size_t payload)
{
	size_t stripeHeight = 4; // for all configuration
	size_t stripePitch = stripeHeight * m_lnkCnt;

	try
	{
		for (size_t ix = 0; ix < m_lnkCnt; ++ix)
		{
			//S640 configure camera
			// configure stripes on grabber data stream
			m_grabberlist[ix]->setString<StreamModule>(std::string("StripeArrangement"), std::string("Geometry_1X_2YM"));
			m_grabberlist[ix]->setInteger<StreamModule>(std::string("LinePitch"), pitch);
			m_grabberlist[ix]->setInteger<StreamModule>(std::string("LineWidth"), pitch);
			m_grabberlist[ix]->setInteger<StreamModule>(std::string("StripeHeight"), stripeHeight);
			m_grabberlist[ix]->setInteger<StreamModule>(std::string("StripePitch"), stripePitch);
			m_grabberlist[ix]->setInteger<StreamModule>(std::string("BlockHeight"), 4); // in every config
			m_grabberlist[ix]->setInteger<StreamModule>(std::string("StripeOffset"), 4 * ix);
		}
		for (uint32_t i = 0 ; i < m_bufferCount; i++)
		{
			m_basebuf[i] = static_cast<uint8_t*>(malloc(payload));
			for (size_t ix = 0; ix < m_lnkCnt; ++ix)
			{
				//S640
				m_grabberlist[ix]->announceAndQueue(UserMemory(m_basebuf[i], payload));
			}
		}
	}
	catch (...)
	{
		return ERROR_PARAMACCESS;
	}
	return SUCCESS;
}

int MultiCXPSource::configS710(size_t pitch, size_t payload)
{
	size_t stripeHeight = 8; // for all configuration
	size_t stripePitch = stripeHeight * m_lnkCnt;

	try
	{
		for (size_t ix = 0; ix < m_lnkCnt; ++ix)
		{
			// 710 configure camera
			// configure stripes on grabber data stream
			m_grabberlist[ix]->setString<StreamModule>(std::string("StripeArrangement"), std::string("Geometry_1X_2YM"));
			m_grabberlist[ix]->setInteger<StreamModule>(std::string("LinePitch"), pitch);
			m_grabberlist[ix]->setInteger<StreamModule>(std::string("LineWidth"), pitch);
			m_grabberlist[ix]->setInteger<StreamModule>(std::string("StripeHeight"), stripeHeight);
			m_grabberlist[ix]->setInteger<StreamModule>(std::string("StripePitch"), stripePitch);
			m_grabberlist[ix]->setInteger<StreamModule>(std::string("BlockHeight"), 8); // in every config
			m_grabberlist[ix]->setInteger<StreamModule>(std::string("StripeOffset"), 8 * ix);
		}
		for (uint32_t i = 0; i < m_bufferCount; i++)
		{
			m_basebuf[i] = static_cast<uint8_t*>(malloc(payload));
			for (size_t ix = 0; ix < m_lnkCnt; ++ix)
			{
				//S710
				m_grabberlist[ix]->announceAndQueue(UserMemory(m_basebuf[i], payload));
			}
		}
	}
	catch (...)
	{
		return ERROR_PARAMACCESS;
	}
	return SUCCESS;
}

int MultiCXPSource::configTopDown()
{
	try
	{
		// configure default top down read out
		m_grabberlist[0]->setString<StreamModule>(std::string("StripeArrangement"), std::string("Geometry_1X_1Y"));
		m_grabberlist[0]->setInteger<StreamModule>(std::string("LinePitch"), 0);
		m_grabberlist[0]->setInteger<StreamModule>(std::string("LineWidth"), 0);
		m_grabberlist[0]->setInteger<StreamModule>(std::string("StripeHeight"), 0);
		m_grabberlist[0]->setInteger<StreamModule>(std::string("StripePitch"), 0);
		m_grabberlist[0]->setInteger<StreamModule>(std::string("BlockHeight"), 0); 
		m_grabberlist[0]->setInteger<StreamModule>(std::string("StripeOffset"), 0);
		
		m_grabberlist[0]->reallocBuffers(m_bufferCount);
	}
	catch (...)
	{
		return ERROR_PARAMACCESS;
	}
	return SUCCESS;;
}

int MultiCXPSource::configGrabbers()
{
	// configure each grabber dma controller
	size_t pitch = m_grabberlist[0]->getWidth() * m_pgentl->imageGetBytesPerPixel(m_grabberlist[0]->getPixelFormat());
	size_t payloadSize = pitch * m_grabberlist[0]->getHeight() * m_lnkCnt;
	int ret = SUCCESS;

	switch (m_cameratype)
	{

	case 1:
		ret = configS990(pitch, payloadSize);
		break;
	case 2:
		ret = configS640(pitch, payloadSize);
		break;
	case 3: 
		ret = configS710(pitch, payloadSize);
		break;
	case 4:
	case 5:
	case 8:
		ret = configTopDown();
		break;
	default:
		ret = ERROR_UNKNOWNDEV;
		break;
	}

	return ret;
}

uint8_t MultiCXPSource::findCameraModel(std::string name, CamNfo& nfo)
{
	uint8_t ret = 0;
	if (name.find("S990") != std::string::npos)
	{
		// this is a S990
		nfo.name = L"S990";
		ret = 1;
		m_pgentl->memento("Found S990");
	}
	else if (name.find("S640") != std::string::npos)
	{
		// this is a S640
		nfo.name = L"S640";
		ret = 2;
		m_pgentl->memento("Found S640");
	}
	else if (name.find("S710") != std::string::npos)
	{
		nfo.name = L"S710";
		ret = 3;
		m_pgentl->memento("Found S710");
	}
	else if (name.find("S210") != std::string::npos)
	{
		nfo.name = L"S210";
		ret = 4;
		m_pgentl->memento("Found S210");
	}
	else if (name.find("S200") != std::string::npos)
	{
		nfo.name = L"S200";
		ret = 5;
		m_pgentl->memento("Found S200");
	}

	else if (!name.compare("Eucalyptus"))
	{
		nfo.name = L"Eucalyptus";
		ret = 8;
		m_pgentl->memento("Found Eucalyptus");
	}
	else
	{
		nfo.name = L"unknown camera";
		ret = 0;
		m_pgentl->memento("Found unknown camera");
		m_pgentl->memento(name);
	}
	return ret;
}

bool MultiCXPSource::checkBank(gc::IF_HANDLE ifh, int bank )
{
	std::vector<std::string> s1{ "A","B","C","D" };
	std::vector<std::string> s2{ "E","F","G","H" };

	std::vector<std::string> val{ "Master", "Extension1", "Extension2", "Extension3" };
	std::vector<std::string> s;
	if (1 == bank)
	{
		s = s1;
	}
	else if (2 == bank)
	{
		s = s2;
	}
	else
		return false;

	std::string status;
	for (int i = 0; i < val.size(); i++)
	{
		m_pgentl->genapiSetString(ifh, "CxpHostConnectionSelector", s[i]);
		status = m_pgentl->genapiGetString(ifh, "CxpConnectionState");
		// cxp detected ?
		if (status.compare("Detected"))
		{
			std::ostringstream os;
			os << "CheckHardware : connector " << s[i] << " not detected";
			m_pgentl->memento(os.str());
			return false;
		}

		status = m_pgentl->genapiGetString(ifh, "CxpDeviceConnectionID");
		if (status.compare(8,(val[i]).length(), val[i]))
		{
			std::ostringstream os;
			os << "CheckHardware : connector " << s[i] << " is not on " << val[i];
			m_pgentl->memento(os.str());
			return false;

		}
	}

	return true;
}

// check the detected hardware and seen if a camera is connected
// 0 => no camera, 1 camera on bank 1, 2 camera on bank 2, 3 camera on bank 1 and 2

int MultiCXPSource::checkHardware(gc::IF_HANDLE ifh)
{
	std::string pc = m_pgentl->genapiGetString(ifh, "ProductCode");
	int firm = (int) m_pgentl->genapiGetInteger(ifh, "FirmwareVariant");
	int count = 0;

	if (pc.compare("PC3602") == 0)
	{
		m_pgentl->memento("CheckHardware : Found Coaxlink Octo"); 
		if (firm != 2)
		{
			m_pgentl->memento("CheckHardware : Found Coaxlink Octo with wrong firmware variant");
			return 0;
		}
		if (checkBank(ifh, 1))
			count |= 0x01;
		if (checkBank(ifh, 2))
			count |= 0x02;
	}
	else if (pc.compare("PC1633") == 0)
	{
		m_pgentl->memento("CheckHardware : Found Coaxlink Quad G3");
		if (firm != 1)
		{
			m_pgentl->memento("CheckHardware : Found Coaxlink Quad G3 with wrong firmware variant");
			return 0;
		}
		if (checkBank(ifh, 1))
			count = 1;
	}
	return count;
}

void MultiCXPSource::convertImage(uint8_t* buf)
{
	FormatConverter::BGR8 bgr(*(m_pconverter), buf,
		m_grabberlist[0]->getPixelFormat(),
		m_sizeX,
		m_sizeY);
	size_t s = m_sizeX * m_sizeY * 3;
	memcpy(m_reco, bgr.getBuffer(), s);
}

int MultiCXPSource::Init(CamNfo& nfo)
{
#ifdef DEMOMODE
	nfo.name = L"Demo";
	nfo.lnkCount = 4;
	m_sizeX = 640;
	m_sizeY = 480;
	m_color = false;
	m_init = true;
	m_buff = (uint8_t*)malloc(m_sizeX * m_sizeY * (m_color ? 3 : 1));
	if (!m_buff)
		return ERROR_NOMEMORY;
	return SUCCESS;
#endif
	nfo.name = L"unknown";
	nfo.lnkCount = 4; 
	// open driver 
	int ret = buildGrabbers();
	if (ret != SUCCESS)
		return ret;

	// what camera do we have ? let check the camera name 
	try
	{
		std::string name = m_grabberlist[0]->getString<RemoteModule>("DeviceModelName");
		m_cameratype = findCameraModel(name, nfo);
		if (!m_cameratype)
		{
			m_pgentl->memento("Found unsupported camera");
			return ERROR_UNKNOWNCAMERA;
		}
		// how many grabber are from this type ?
		// on true for multi bank grabbers
		size_t s = m_grabberlist.size();
		if (m_cameratype == 3 || m_cameratype == 2 || m_cameratype == 1)
		{

			if (s > 1)
			{
				for (int i = (int)s - 1; i > -1; i--)
				{
					CamNfo nfo;
					std::string name = m_grabberlist[i]->getString<RemoteModule>("DeviceModelName");
					uint8_t type = findCameraModel(name, nfo);
					if (type != m_cameratype)
					{
						// this camera is not what we expect , remove it from the list
						codT* g = m_grabberlist[i];
						delete g;
						g = nullptr;
						m_grabberlist.erase(m_grabberlist.begin() + i);
					}
				}

			}
		}
		else
		{
			// we have a 1 bank camera
			// keep only the first camera
			if (s > 1)
			{
				for (int i = (int)s - 1; i > -1; i--)
				{
					// remove camera from the list
					codT* g = m_grabberlist[i];
					delete g;
					g = nullptr;
					m_grabberlist.erase(m_grabberlist.begin() + i);
					
				}
			}
		}

		// we should have a least one grabber left
		s = m_grabberlist.size();
		if (s < 1)
		{
			// some thing went wrong
			m_pgentl->memento("Failed : no more compatible camera ");
			return ERROR_UNKNOWNCAMERA;
		}

	}
	catch(...)
	{
		return ERROR_UNKNOWNCAMERA;
	}
	// set the grabber in order => MasterHostLink ID is no working as it is not taken into account by the camera ... :(
	// Check MasterHostLinkID of the camera 
	/*try
	{
		size_t s = m_grabberlist.size();
		size_t linkid = s < 4 ? s : 4;		// camera have max 4 banks
		
		// looking for 
		for (int i = 0; i < linkid; i++)
		{
			for (int j = i; j < s; j++)
			{
				int64_t id = m_grabberlist[j]->getInteger<RemoteModule>("MasterHostLinkID"); // id is going from 1->4
				if (id == i+1)	// ok found the id we where looking for
				{
					// is it properly positioned ?
					if (i != j)
					{
						// no we need to swap it  ...
						std::swap(m_grabberlist[j], m_grabberlist[i]);
					}
					else
						break;
				}
			}
		}
		
	}
	catch (const std::exception&)
	{

	}*/
	// only change if we have a multi channel camera

	if (m_cameratype == 3 || m_cameratype == 2 || m_cameratype == 1)		// we have a real camera 
	{
		// configure for acquisition
		try
		{
			m_grabberlist[0]->setString<RemoteModule>("TriggerMode", "TriggerModeOn");   // camera in triggered mode
			m_grabberlist[0]->setString<RemoteModule>("TriggerSource", "SWTRIGGER");     // source of trigger CXP

			m_grabberlist[0]->setString<DeviceModule>("CameraControlMethod", "RC");      // tell grabber 0 to send trigger
			m_grabberlist[0]->setFloat<DeviceModule>("CycleMinimumPeriod", 20000.0);  // set the trigger rate to 50 Hz
			m_grabberlist[0]->setFloat<DeviceModule>("ExposureRecoveryTime", 10.0);
			m_grabberlist[0]->setFloat<DeviceModule>("ExposureTime", 10.0);
			m_grabberlist[0]->setFloat<DeviceModule>("StrobeDuration", 10.0);
			m_grabberlist[0]->setString<DeviceModule>("ExposureReadoutOverlap", "True"); // camera needs 2 trigger to start
		}
		catch (...)
		{
			m_pgentl->memento("Failed to set camera configuration");
			return ERROR_CAMERASETTING;
		}
		// how many grabber ?
		size_t s = m_grabberlist.size();
		if (s >= 4)
			s = 4;
		else if (s >= 2)
			s = 2;
		else
			s = 1;			// for a single camera only 4,2 or 1 grabber can be used
		nfo.lnkCount = (uint32_t)s;
		m_lnkCnt = nfo.lnkCount;

		// try to set the camera according to the grabber that have been found
		try
		{
			switch (s)
			{
			case 4:
				m_grabberlist[0]->setString<RemoteModule>("Banks", "Banks_ABCD");
				break;
			case 2:
				m_grabberlist[0]->setString<RemoteModule>("Banks", "Banks_AB");
				break;
			case 1:
			default:
				m_grabberlist[0]->setString<RemoteModule>("Banks", "Banks_A");
				break;
			}
		}
		catch (...)
		{
			m_pgentl->memento("Failed to set Bank camera configuration");
			return ERROR_CAMERASETTING;
		}
		// config the grabbers
		ret = configGrabbers();
		if (ret != SUCCESS)
			return ret;
/*
		m_grabberlist[0]->setInteger<RemoteModule>("Width", 640);     // Special for demo
		m_grabberlist[0]->setInteger<RemoteModule>("Height", 256 / nfo.lnkCount);     // Special for demo*/
		m_grabberlist[0]->setFloat<RemoteModule>("ExposureTime", 5000.0);
	}
	else
	{
		// we have a simulator
		nfo.lnkCount = 1;
		m_lnkCnt = nfo.lnkCount;
		m_grabberlist[0]->reallocBuffers(CXPBUFCOUNT);

	}
	// fetch camera info
	//std::string name = m_grabberlist[0]->getString<DeviceModule>("DeviceModelName");
	//nfo.name = CString(name.c_str());
	m_sizeX = m_grabberlist[0]->getWidth();
	m_sizeY = m_grabberlist[0]->getHeight() * nfo.lnkCount;


	m_buff = (uint8_t*) malloc(m_sizeX * m_sizeY *  3 );
	if (!m_buff)
		return ERROR_NOMEMORY;
	m_reco = (uint8_t*)malloc(m_sizeX * m_sizeY * 3);
	if (!m_reco)
		return ERROR_NOMEMORY;

	// start acquistion statistic
	m_grabberlist[0]->execute<StreamModule>(std::string("StatisticsStartSampling"));

	m_init = true;
	return SUCCESS;
}

int MultiCXPSource::Start()
{
	if (m_brun)
		return SUCCESS;
#ifdef DEMOMODE
	m_brun = true;
	return SUCCESS;
#endif
	// reset all stream counters
	m_grabberlist[0]->execute<StreamModule>("EventCountResetAll");
	
	// start acq thread
	m_brun = true;
	m_acqthread = new std::thread(acqThread, this);


	// start all grabbers in inverse order
	try
	{
		for (int i = m_lnkCnt - 1; i > -1; i--)
		{
			m_grabberlist[i]->start();
		}
	}
	catch (...)
	{
		m_pgentl->memento("Failed to start the acquisition");
		return ERROR_START;
	}


	return SUCCESS;
}

int MultiCXPSource::Record()
{
	if (!m_brun)
		Start();		// did we start the acquisition?
	if (!m_bRec)
	{					// are we already recording
		if (nullptr == m_mm)
		{
			// todo make sure we can allocate the memory with no error
			m_mm = new MemoryManager();
			unsigned int bpp = m_pgentl->imageGetBytesPerPixel(m_grabberlist[0]->getPixelFormat());
			m_buffcount = m_mm->Init(0.75, m_sizeX * m_sizeY * bpp);
			
		}
		m_mm->StartStoring();
		m_bRec = true;
		m_bRecDone = false;
		// todo start time stamping 

	}
	return (int)m_buffcount;
}
bool MultiCXPSource::StopRecord()
{
	if (!m_bRec)
		return false; // nothing to do
	if (nullptr != m_mm)
	{
		m_mm->StopStoring();
		m_bRec = false;
		m_bRecDone = true;
	}
	return true;
}

bool MultiCXPSource::IsRecording()
{
	return m_bRec;
}

size_t MultiCXPSource::GetRecCount()
{
	if (!m_bRec)
		return 0;
	if (nullptr != m_mm)
		return m_mm->IsStoring();
	else
		return 0;
}

int MultiCXPSource::Stop()
{
	if (!m_brun)
		return SUCCESS;
#ifdef DEMOMODE
	m_brun = false;
	return SUCCESS;
#endif
	m_brun = false;
	m_acqthread->join();

	// stop all grabbers in inverse order
	try
	{
		for (unsigned int i = 0; i < m_lnkCnt; i++)
		{
			m_grabberlist[i]->stop();
			m_grabberlist[i]->resetBufferQueue();
			m_grabberlist[i]->flushBuffers();
		}
	}
	catch (...)
	{
		m_pgentl->memento("Failed to stop the acquisition");
		return ERROR_STOP;
	}
	return SUCCESS;
}

int MultiCXPSource::GetImage(UINT8** data)
{
	if (!m_brun)
		return ERROR_NOSTARTED;
#ifdef DEMOMODE
	
	static char offset = 0;
	uint8_t* px = m_buff;
	for (int j = 0; j < m_sizeY; j++)
	{
		for (int i = 0; i < m_sizeX; i++)
		{
			if (m_color)
			{
				*px = 255 - ((uint8_t)(offset + i + j));
				px++;				
				*px = (uint8_t)(offset + i + j);
				px++;				
				*px = (uint8_t)(128 - (uint8_t)(offset + i + j));
				px++;
			}
			else
			{
				*px = (uint8_t)(offset + i + j);
				px++;
			}
		}
	}
	offset++;
	*data = m_buff;
	if (m_bRec)
	{
		LARGE_INTEGER li;
		QueryPerformanceCounter(&li);
		if (!m_mm->SetBuffer(m_buff, li.QuadPart))
		{
			m_bRec = false;
			m_bRecDone = true;
			m_mm->StopStoring();
		}
	}
	return SUCCESS;
#endif

	m_copybuf = true;
	uint32_t timeout = 0;
	while (m_copybuf)
	{
		Sleep(10);
		timeout++;
		if (timeout > 150)		// 1.5 sec timeout
			return ERROR_ACQTIMEOUT;
	}
	*data = m_buff;
	return SUCCESS;
}

int MultiCXPSource::GetImageInfo(ImgNfo& nfo)
{
	if (!m_init)
		return ERROR_NOINIT;
	nfo.sizeX = m_sizeX;
	nfo.sizeY = m_sizeY;

	return SUCCESS;
}

int MultiCXPSource::GetStat(GrabStat& stat)
{
	if (!m_init)
		return ERROR_NOINIT;
#ifdef DEMOMODE
	stat.fps = m_fps;
	stat.mbps = (m_sizeX * m_sizeY * m_fps) / (1024.0*1024.0);
	stat.lostframes = 0;
	return SUCCESS;
#endif
	try
	{
		m_grabberlist[0]->setString<StreamModule>("StatisticsSamplingSelector", "LastSecond");
		stat.fps = m_grabberlist[0]->getFloat<StreamModule>("StatisticsFrameRate");
		stat.mbps = (double)(m_lnkCnt * m_grabberlist[0]->getFloat<StreamModule>("StatisticsDataRate")) /(double)(1024.0*1024.0);

		m_grabberlist[0]->setString<StreamModule>("EventSelector", "RejectedFrame"); // find how many drop frame we got
		stat.lostframes = m_grabberlist[0]->getInteger<StreamModule>("EventCount");
	}
	catch (...)
	{
		return ERROR_PARAMACCESS;
	}
	return SUCCESS;
}

int MultiCXPSource::SetFps(double fps)
{
	if (fps < 0.1)
		return ERROR_PARAMOUTOFRANGE;
	if (fps > 297000.0)
		return ERROR_PARAMOUTOFRANGE;
	if (!m_init)
		return ERROR_NOINIT;
#ifdef DEMOMODE
	m_fps = fps;
	return SUCCESS;
#endif
	// TODO check fps related to camera model, resolution and number of links 
	double val = 1000000.0 / fps; // period in us
	try
	{
		m_grabberlist[0]->setFloat<DeviceModule>("CycleMinimumPeriod", val);
	}
	catch (...)
	{
		return ERROR_PARAMACCESS;
	}
	return SUCCESS;
}


int MultiCXPSource::SetExposure(double exp)
{
	if (!m_init)
		return ERROR_NOINIT;
#ifdef DEMOMODE
	exp = m_exp;
	return SUCCESS;
#endif
	try 
	{
		m_grabberlist[0]->setFloat<RemoteModule>("ExposureTime", exp);
	}
	catch(...)
	{
		return ERROR_PARAMACCESS;
	}
	return SUCCESS;
}

int MultiCXPSource::GetExposure(double& exp)
{
	if (!m_init)
		return ERROR_NOINIT;
#ifdef DEMOMODE
	exp = m_exp;
	return SUCCESS;
#endif
	try
	{
		exp = m_grabberlist[0]->getFloat<RemoteModule>("ExposureTime");
	}
	catch (...)
	{
		return ERROR_PARAMACCESS;
	}
	return SUCCESS;
}

int MultiCXPSource::SetResolution(size_t X, size_t Y)
{
	if (!m_init)
		return ERROR_NOINIT;
#ifdef DEMOMODE
	if (m_sizeX == X && m_sizeY == Y)
		return SUCCESS;
	if (m_buff)
	{
		free(m_buff);
	}
	m_buff = (uint8_t*)malloc(m_sizeX * m_sizeY * (m_color ? 3 : 1));
	if (!m_buff)
		return ERROR_NOMEMORY;
	return SUCCESS;
#endif
	return ERROR_NOTIMPLEMENTED;
}

int MultiCXPSource::GetRecordedRange(uint64_t& buffercount, uint64_t& start, uint64_t& end)
{
	if (!m_init)
		return ERROR_NOINIT;
	uint64_t ms;
	if (!m_mm->GetRange(ms, start, end))
		return ERROR_PARAMOUTOFRANGE;
	buffercount = m_buffcount;
	return SUCCESS;
}

int MultiCXPSource::GetRecordImageAt(UINT8** data, uint64_t& at)
{
	if(!m_bRecDone)
		return ERROR_PARAMOUTOFRANGE;
	UINT8* buff;
	if (!m_mm->SeekBuffer((void**)&buff, at, at))
		return ERROR_PARAMACCESS;
	convertImage(buff);
	*data = m_reco;
	return SUCCESS;
}

int MultiCXPSource::GetRecordRawAt(UINT8** data, uint64_t& at)
{
	if (!m_bRecDone)
		return ERROR_PARAMOUTOFRANGE;
	if (!m_mm->SeekBuffer((void**)data, at, at))
		return ERROR_PARAMACCESS;
	return SUCCESS;
}

int MultiCXPSource::GetRecordedImageNext(UINT8** data, uint64_t& at)
{
	if (!m_bRecDone)
		return ERROR_PARAMOUTOFRANGE;
	UINT8* buff;
	if (!m_mm->GetNextBuffer((void**)&buff, at))
		return ERROR_PARAMACCESS;
	convertImage(buff);
	*data = m_reco;
	return SUCCESS;

}

int MultiCXPSource::GetRecordedRawNext(UINT8** data, uint64_t& at)
{
	if (!m_bRecDone)
		return ERROR_PARAMOUTOFRANGE;
	if (!m_mm->GetNextBuffer((void**)data, at))
		return ERROR_PARAMACCESS;
	return SUCCESS;
}

int MultiCXPSource::GetRecordedImageNextEx(UINT8** data, int skip, uint64_t& at)
{
	if (!m_bRecDone)
		return ERROR_PARAMOUTOFRANGE;
	UINT8* buff;
	if (!m_mm->GetNextBufferEx((void**)&buff, skip, at))
		return ERROR_PARAMACCESS;
	convertImage(buff);
	*data = m_reco;
	return SUCCESS;

}

int MultiCXPSource::SaveImage(void* buf, std::string pathname)
{
	if (!m_init)
		return ERROR_NOINIT;

	ge::ImageConvertInput input = { static_cast<int>(m_sizeX), static_cast<int>(m_sizeY), buf, "Mono8", };

	m_pgentl->imageSaveToDisk(input, pathname);

	return 0;
}
