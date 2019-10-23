#include "pch.h"
#include "MultiCXPSource.h"



void acqThread(MultiCXPSource* source)
{
	size_t s = source->m_sizeX * source->m_sizeY * (source->m_color ? 3 : 1);
	std::vector<std::pair<Buffer, codT*>> bufnfo;
	while (source->m_brun)
	{

		try
		{
			// get buffers from output of each grabber (pop out of output queue)
			for (codT* g : source->m_grabberlist)
			{
				Buffer b = (*g).pop((uint64_t)5000000);
				std::pair<Buffer, codT*> p(b,g);
				bufnfo.push_back(p);
			}

			// get buffer this first grabber hold alway ( for the S990 and S640) the pointer to the first pixel
			uint8_t* imagePointer = ((bufnfo[0]).first).getInfo<uint8_t*>(*(bufnfo[0].second),GenTL::BUFFER_INFO_BASE);

			// preview image needed
			if (source->m_copybuf)
			{
				// convert the image to the right format


				memcpy(source->m_buff, imagePointer, s);
				source->m_copybuf = false;
			}

			// return eeach buffer to its own grabber ( push to input queue)
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
	}
	catch (...)
	{
		// access the driver failed can do nothing 
		return ERROR_NODRIVER;
	}
	m_pgentl->memento(std::string("Source open driver"));

	// find how many device we have
	int devCnt = 0;
	EGrabber<CallbackOnDemand>* grab;
	try
	{
		gc::TL_HANDLE tlh = m_pgentl->tlOpen();
		int num = m_pgentl->tlGetNumInterfaces(tlh);						// find interfaces num


		// per interface
		for (int iid = 0; iid < num; iid++)								// get all interface
		{
			std::string ifid = m_pgentl->tlGetInterfaceID(tlh, iid);
			gc::IF_HANDLE ifh = m_pgentl->tlOpenInterface(tlh, ifid);

			m_cameracountlist.push_back(m_pgentl->ifGetNumDevices(ifh));		// get num devices

			m_pgentl->ifClose(ifh);										// free Interfaces
			devCnt++;
		}
		m_pgentl->tlClose(tlh);											// close all handles  as EGrabber need them
		m_pgentl->memento(std::string("GetGENTL: retrieved board/camera count"));


		//per device
		for (int iid = 0; iid < devCnt; iid++)
		{

			for (int did = 0; did < m_cameracountlist[iid]; did++)
			{
				std::ostringstream os;
				os << "----- Create Grabber " << (iid + 1) << "/" << devCnt << " , " << (did + 1) << "/" << m_cameracountlist[iid] << std::endl;
				m_pgentl->memento(os.str());
				grab = new EGrabber<CallbackOnDemand>(*m_pgentl, iid, did);
				m_grabberlist.push_back(grab);  // save devices
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
	size_t stripeHeight = 4; // for all configuration
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
			uint8_t* base = static_cast<uint8_t*>(malloc(payload));
			for (size_t ix = 0; ix < m_lnkCnt; ++ix)
			{
				size_t offset = pitch * stripeHeight * ix;
				//S990
				m_grabberlist[ix]->announceAndQueue(UserMemory(base + offset, payload - offset));
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
			uint8_t* base = static_cast<uint8_t*>(malloc(payload));
			for (size_t ix = 0; ix < m_lnkCnt; ++ix)
			{
				//S640
				m_grabberlist[ix]->announceAndQueue(UserMemory(base, payload));
			}
		}
	}
	catch (...)
	{
		return ERROR_PARAMACCESS;
	}
	return SUCCESS;
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

	default:
		ret = ERROR_UNKNOWNDEV;
		break;
	}

	return ret;
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
	nfo.name = L" S640";
	nfo.lnkCount = 4; 
	// open driver 
	int ret = buildGrabbers();
	if (ret != SUCCESS)
		return ret;

	// TODO : remove all frame grabber that are not QUAD or OCTO


	// TODO : check that all grabber are connected to the same camera

	// TODO : order the grabber 1 Bank -> Bank 4

	
	// configure for acquisition
	m_grabberlist[0]->setString<RemoteModule>(std::string("TriggerMode"), std::string("TriggerModeOn"));   // camera in triggered mode
	m_grabberlist[0]->setString<RemoteModule>(std::string("TriggerSource"), std::string("SWTRIGGER"));     // source of trigger CXP
	m_grabberlist[0]->setString<DeviceModule>(std::string("CameraControlMethod"), std::string("RC"));      // tell grabber 0 to send trigger
	m_grabberlist[0]->setFloat<DeviceModule>(std::string("CycleMinimumPeriod"), 20000.0);  // set the trigger rate to 50 Hz
	m_grabberlist[0]->setString<DeviceModule>(std::string("ExposureReadoutOverlap"), std::string("True")); // camera needs 2 trigger to start
	
	// how many grabber ?
	nfo.lnkCount = (uint32_t)m_grabberlist.size();
	m_lnkCnt = nfo.lnkCount;

	// config the grabbers
	ret = configGrabbers();
	if (ret != SUCCESS)
		return ret;

	// fetch camera info
	std::string name = m_grabberlist[0]->getString<DeviceModule>(std::string("DeviceModelName"));
	nfo.name = CString(name.c_str());
	m_sizeX = m_grabberlist[0]->getWidth();
	m_sizeY = m_grabberlist[0]->getHeight() * nfo.lnkCount;
	
	m_buff = (uint8_t*) malloc(m_sizeX * m_sizeY * (m_color ? 3 : 1));
	if (!m_buff)
		return ERROR_NOMEMORY;
	std::string color = m_grabberlist[0]->getString<RemoteModule>(std::string("PixelFormat"));
	if (color.at(0) == 'M')
		m_color = false;
	else
		m_color = true;

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
	m_grabberlist[0]->execute<StreamModule>(std::string("EventCountResetAll"));
	
	// start acq thread
	m_brun = true;
	m_acqthread = new std::thread(acqThread, this);


	// start all grabbers

	return SUCCESS;
}

int MultiCXPSource::Record()
{
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
	nfo.color = m_color;
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
	stat.mbps = m_sizeX * m_sizeY * m_fps;
	stat.lostframes = 0;
	return SUCCESS;
#endif
	try
	{
		m_grabberlist[0]->setString<StreamModule>(std::string("StatisticsSamplingSelector"), std::string("LastSecond"));
		stat.fps = m_grabberlist[0]->getFloat<StreamModule>(std::string("StatisticsFrameRate"));
		stat.mbps = m_lnkCnt * m_grabberlist[0]->getFloat<StreamModule>(std::string("StatisticsDataRate"));

		m_grabberlist[0]->setString<StreamModule>(std::string("EventSelector"), std::string("RejectedFrame")); // find how many drop frame we got
		stat.lostframes = m_grabberlist[0]->getInteger<StreamModule>(std::string("EventCount"));
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
		m_grabberlist[0]->setFloat<DeviceModule>(std::string("CycleMinimumPeriod"), val);
	}
	catch (...)
	{
		return ERROR_PARAMACCESS;
	}
	return SUCCESS;
}

int MultiCXPSource::GetFps(double& fps)
{
	if (!m_init)
		return ERROR_NOINIT;
#ifdef DEMOMODE
	fps = m_fps;
	return SUCCESS;
#endif
	try
	{
		double val = m_grabberlist[0]->getFloat<DeviceModule>(std::string("CycleMinimumPeriod"));
		fps = 1000000 / val;
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
		m_grabberlist[0]->setFloat<RemoteModule>(std::string("ExposureTime"), exp);
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
		exp = m_grabberlist[0]->getFloat<RemoteModule>(std::string("ExposureTime"));
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
