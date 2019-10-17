#include "pch.h"
#include "MultiCXPSource.h"


void acqThread(MultiCXPSource* source)
{
	while (source->m_brun)
	{
		try
		{
			
			
			// preview image needed
			if (source->m_copybuf)
			{
				memcpy(source->m_buff, buf_in, size);
				source->m_copybuf = false;
			}
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
	m_pgentl->memento("Source open driver");

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
		m_pgentl->memento("GetGENTL: retrieved board/camera count");


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

int MultiCXPSource::configS990(size_t pitch)
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
			m_grabberlist[ix]->setInteger<StreamModule>("LinePitch", pitch);
			m_grabberlist[ix]->setInteger<StreamModule>("LineWidth", pitch);
			m_grabberlist[ix]->setInteger<StreamModule>("StripeHeight", stripeHeight);
			m_grabberlist[ix]->setInteger<StreamModule>("StripePitch", stripePitch);
		}
	}
	catch (...)
	{
		return ERROR_PARAMACCESS;
	}
	return SUCCESS;
}

int MultiCXPSource::configS640(size_t pitch)
{
	size_t stripeHeight = 4; // for all configuration
	size_t stripePitch = stripeHeight * m_lnkCnt;

	try
	{
		for (size_t ix = 0; ix < m_lnkCnt; ++ix)
		{
			//S640 configure camera
			// configure stripes on grabber data stream
			m_grabberlist[ix]->setString<StreamModule>("StripeArrangement", "Geometry_1X_2YM");
			m_grabberlist[ix]->setInteger<StreamModule>("LinePitch", pitch);
			m_grabberlist[ix]->setInteger<StreamModule>("LineWidth", pitch);
			m_grabberlist[ix]->setInteger<StreamModule>("StripeHeight", stripeHeight);
			m_grabberlist[ix]->setInteger<StreamModule>("StripePitch", stripePitch);
			m_grabberlist[ix]->setInteger<StreamModule>("BlockHeight", 4); // in every config
			m_grabberlist[ix]->setInteger<StreamModule>("StripeOffset", 4 * ix);
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
		ret = configS990(pitch);
		break;
	case 2:
		ret = configS640(pitch);
		break;

	default:
		ret = ERROR_UNKNOWNDEV;
		break;
	}

	return ret;
}

int MultiCXPSource::Init(CamNfo& nfo)
{
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
	m_grabberlist[0]->setString<RemoteModule>("TriggerMode", "TriggerModeOn");   // camera in triggered mode
	m_grabberlist[0]->setString<RemoteModule>("TriggerSource", "SWTRIGGER");     // source of trigger CXP
	m_grabberlist[0]->setString<DeviceModule>("CameraControlMethod", "RC");      // tell grabber 0 to send trigger
	m_grabberlist[0]->setString<DeviceModule>("CycleMinimumPeriod", "20000.0");  // set the trigger rate to 50 Hz
	m_grabberlist[0]->setString<DeviceModule>("ExposureReadoutOverlap", "True"); // camera needs 2 trigger to start
	
	// how many grabber ?
	nfo.lnkCount = m_grabberlist.size();
	m_lnkCnt = nfo.lnkCount;

	// config the grabbers
	ret = configGrabbers();
	if (ret != SUCCESS)
		return ret;

	// fetch camera info
	std::string name = m_grabberlist[0]->getString<DeviceModule>("DeviceModelName");
	nfo.name = CString(name.c_str());
	m_sizeX = m_grabberlist[0]->getWidth();
	m_sizeY = m_grabberlist[0]->getHeight() * nfo.lnkCount;
	
	std::string color = m_grabberlist[0]->getString<RemoteModule>("PixelFormat");
	if (color.at(0) == 'M')
		m_color = false;
	else
		m_color = true;

	// start acquistion statistic
	m_grabberlist[0]->execute<StreamModule>("StatisticsStartSampling");

	return SUCCESS;
}

int MultiCXPSource::Start()
{
	if (m_brun)
		return SUCCESS;
	// reset all stream counters
	m_grabberlist[0]->execute<StreamModule>("EventCountResetAll"); 
	
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
	return 0;
}

int MultiCXPSource::GetImage(UINT8* data)
{
	if (!m_brun)
		return ERROR_NOSTARTED;

	m_copybuf = true;
	uint32_t timeout = 0;
	while (m_copybuf)
	{
		Sleep(10);
		timeout++;
		if (timeout > 150)		// 1.5 sec timeout
			return ERROR_ACQTIMEOUT;
	}
	data = m_buff;
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
	try
	{
		m_grabberlist[0]->setString<StreamModule>("StatisticsSamplingSelector", "LastSecond");
		stat.fps = m_grabberlist[0]->getFloat<StreamModule>("StatisticsFrameRate");
		stat.mbps = m_lnkCnt * m_grabberlist[0]->getFloat<StreamModule>("StatisticsDataRate");

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

int MultiCXPSource::GetFps(double& fps)
{
	try
	{
		double val = m_grabberlist[0]->getFloat<DeviceModule>("CycleMinimumPeriod");
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
	return 0;
}
