#include "pch.h"
#include "MultiCXPSource.h"

int MultiCXPSource::Init(CamNfo& nfo)
{
	nfo.name = L" S640";
	nfo.lnkCount = 4; 
	// open driver 
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

	// TODO : remove all frame grabber that are not QUAD or OCTO


	// TODO : check that all grabber are connected to the same camera

	// TODO : order the grabber 1 Bank -> Bank 4

	// TODO : init all the grabber to accomodate the coneected camera

	nfo.lnkCount = m_grabberlist.size();
	std::string name = m_grabberlist[0]->getString<DeviceModule>("DeviceModelName");
	nfo.name = CString(name.c_str());
	return SUCCESS;
}

int MultiCXPSource::Start()
{
	return 0;
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
	return 0;
}

int MultiCXPSource::GetImageInfo(ImgNfo& nfo)
{
	return 0;
}

int MultiCXPSource::GetStat(GrabStat& stat)
{
	return 0;
}

int MultiCXPSource::SetFps(size_t fps)
{
	return 0;
}

int MultiCXPSource::SetExposure(size_t exp)
{
	return 0;
}

int MultiCXPSource::SetResolution(size_t X, size_t Y)
{
	return 0;
}
