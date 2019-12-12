#pragma once

#include "c:\Program Files\Euresys\Coaxlink\include\EGrabber.h"
#include "error.h"
#include <thread>
#include <string>
#include <vector>
#include <algorithm>

#define DEMOMODE 1

using namespace Euresys;

class MemoryManager
{
private:
	std::vector<void*>		m_buffers;		// allocated buffers
	std::vector<std::pair<uint64_t, void*>> m_data;
	int m_currentBuffer;
	size_t m_bufferSize;
	bool m_isStoring;
public:
	MemoryManager();
	~MemoryManager();

	// return how many buffer could be allocated
	size_t Init(float usage, size_t buffersize);
	// store a buffer
	bool SetBuffer(void* buffer, uint64_t time);
	bool GetBuffer(void** buffer, int64_t increment_us, uint64_t& time);
	bool SeekBuffer(void** buffer, int64_t frombegin_us, uint64_t& time);

	bool GetNextBuffer(void** buffer, uint64_t& time);
	bool StartStoring();
	bool StopStoring();
	bool GetRange(uint64_t& ms, uint64_t& begin, uint64_t& end);
	bool IsStoring();
};

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
	MemoryManager* m_mm;
	bool m_bRec;
	size_t m_buffcount;

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
#ifdef DEMOMODE
	double  m_exp;
	double	m_fps;
#endif
public :	
	MultiCXPSource() :
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
		, m_bRec(false)
		, m_mm(nullptr)
		, m_buffcount(0)
	{
		// constructor
		m_cameracountlist.clear();
		m_grabberlist.clear();
#ifdef DEMOMODE
		m_exp = 10;
		m_fps = 40;
#endif

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
		if (m_buff)
		{
			free(m_buff);
			m_buff = nullptr;
		}
	}


	int Init(CamNfo& nfo);
	int Start();
	int Record();
	int Stop();
	int GetImage(UINT8 ** data);
	int GetImageInfo(ImgNfo& nfo);
	int GetStat(GrabStat& stat);
	int SetFps(double fps);
	int SetExposure(double exp);
	int GetExposure(double& exp);
	int SetResolution(size_t X, size_t Y);




};

