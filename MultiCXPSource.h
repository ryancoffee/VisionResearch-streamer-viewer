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
	double	fps;
	double	mbps;
	size_t	lostframes;
};

class MultiCXPSource
{

private:
	size_t m_sizeX;
	size_t m_sizeY;
	bool m_color;
	bool m_init;
	uint32_t m_lnkCnt;
	size_t m_lostCnt;
	EGenTL* m_pgentl;

	std::vector<EGrabber<CallbackOnDemand>*> m_grabberlist;
	std::vector<int> m_cameracountlist;
public :	
	MultiCXPSource():
	m_pgentl(nullptr)
	, m_sizeX(0)
	, m_sizeY(0)
	, m_color(false)
	, m_init(false)
	, m_lnkCnt(1)
	, m_lostCnt(0)
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

