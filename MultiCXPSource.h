#pragma once

#include "c:\Program Files\Euresys\Coaxlink\include\EGrabber.h"
#include "error.h"
#include <thread>

using namespace Euresys;

struct ImgNfo
{
	size_t	sizeX=0;
	size_t	sizeY=0;
	bool	color=false;
};

struct CamNfo
{
	CString name;
	UINT32 lnkCount = 1;
};

struct GrabStat
{
	double	fps;
	double	mbps;
	size_t	lostframes;
};

typedef EGrabber<CallbackOnDemand> codT;

class MultiCXPSource
{
public:
	volatile bool m_brun;												// thread runing
	std::vector<codT*> m_grabberlist;				// grabber list
	volatile bool m_copybuf;											// copy the buffer
	uint8_t* m_buff;													// buffer that contain a image copy
	size_t m_sizeX;
	size_t m_sizeY;
	bool m_color;
	uint32_t m_lnkCnt;

private:

	bool m_init;
	EGenTL* m_pgentl;
	std::thread* m_acqthread;
	std::vector<int> m_cameracountlist;
	// 0 unknow, 1 S990, 2 S640, 3 S710
	uint8_t m_cameratype; 
	uint32_t m_bufferCount;

	int buildGrabbers();
	int configS990(size_t pitch, size_t payload);
	int configS640(size_t pitch, size_t payload);
	int configGrabbers();


public :	
	MultiCXPSource():
	m_pgentl(nullptr)
	, m_sizeX(0)
	, m_sizeY(0)
	, m_color(false)
	, m_init(false)
	, m_lnkCnt(1)
	, m_buff(nullptr)
	, m_copybuf(false)
	, m_acqthread(nullptr)
	, m_brun(false)
	, m_cameratype(0)
	, m_bufferCount(20)
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
	int SetFps(double fps);
	int GetFps(double& fps);
	int SetExposure(double exp);
	int GetExposure(double& exp);
	int SetResolution(size_t X, size_t Y);




};

