#pragma once

#include "c:\Program Files\Euresys\Coaxlink\include\EGrabber.h"
#include "error.h"

using namespace Euresys;

struct ImgNfo
{
	size_t	sizeX;
	size_t	sizeY;
	bool	color;
};

struct CamNfo
{
	CString name;
	UINT32 lnkCount;
};

struct GrabStat
{
	size_t	fps;
	size_t	mbps;
	size_t	lostframes;
};

class MultiCXPSource
{

private:
	EGenTL* m_pgentl;
	std::vector<EGrabber<CallbackOnDemand>*> m_grabberlist;
	std::vector<int> m_cameracountlist;
public :	
	MultiCXPSource():
	m_pgentl(nullptr)
	{
		// constructor
		m_cameracountlist.clear();
		m_grabberlist.clear();

	}
	~MultiCXPSource()
	{
		// destructor

		// device list
		for (EGrabber<CallbackOnDemand>* &grab : m_grabberlist)
		{
			grab->stop();
			delete (grab);
			grab = nullptr;
		}
		m_grabberlist.clear();

		// gentl
		if (m_pgentl)
		{
			delete m_pgentl;
			m_pgentl = nullptr;
		}
	}


	int Init(CamNfo& nfo);
	int Start();
	int Record();
	int Stop();
	int GetImage(UINT8 * data);
	int GetImageInfo(ImgNfo& nfo);
	int GetStat(GrabStat& stat);
	int SetFps(size_t fps);
	int SetExposure(size_t exp);
	int SetResolution(size_t X, size_t Y);

};

