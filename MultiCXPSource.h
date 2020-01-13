#pragma once

#include "c:\Program Files\Euresys\Coaxlink\include\EGrabber.h"
#include "c:\Program Files\Euresys\Coaxlink\include\FormatConverter.h"
#include "error.h"
#include <thread>
#include <string>
#include <vector>
#include <array>
#include <algorithm>

//#define DEMOMODE 1
#define CXPBUFCOUNT		20
using namespace Euresys;

typedef std::pair<uint64_t, void*> dpair;

class MemoryManager
{
private:
	std::vector<void*>		m_buffers;		// allocated buffers
	std::vector<dpair> m_data;
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
	uint32_t m_lnkCnt;
	MemoryManager* m_mm;
	bool m_bRec;
	size_t m_buffcount;
	FormatConverter* m_pconverter;

private:

	bool m_init;
	EGenTL* m_pgentl;
	std::thread* m_acqthread;
	std::vector<std::pair<int, int>> m_cameraList;
	// 0 unknow, 1 S990, 2 S640, 3 S710
	uint8_t m_cameratype; 
	uint32_t m_bufferCount;
	bool m_bRecDone;
	std::array<uint8_t*, CXPBUFCOUNT> m_basebuf;

	int buildGrabbers();
	int configS990(size_t pitch, size_t payload);
	int configS640(size_t pitch, size_t payload);
	int configGrabbers();
	uint8_t findCameraModel(std::string name, CamNfo& nfo);
	bool checkBank(gc::IF_HANDLE ifh, int bank);
	int checkHardware(gc::IF_HANDLE ifh); // Check if we have an Octo or a quad G3 and if camera is properly connected

#ifdef DEMOMODE
	double  m_exp;
	double	m_fps;
#endif
public :	
	MultiCXPSource() :
		m_pgentl(nullptr)
		, m_pconverter(nullptr)
		, m_sizeX(0)
		, m_sizeY(0)
		, m_init(false)
		, m_lnkCnt(1)
		, m_buff(nullptr)
		, m_copybuf(false)
		, m_acqthread(nullptr)
		, m_brun(false)
		, m_cameratype(0)
		, m_bufferCount(CXPBUFCOUNT)
		, m_bRec(false)
		, m_mm(nullptr)
		, m_buffcount(0)
		, m_bRecDone(false)
	{
		// constructor
		m_grabberlist.clear();
		m_cameraList.clear();
		m_basebuf.fill(nullptr);


#ifdef DEMOMODE
		m_exp = 10;
		m_fps = 40;
#endif

	}
	~MultiCXPSource()
	{
		// destructor
		m_brun = false;
		// device list
		for (EGrabber<CallbackOnDemand>* &grab : m_grabberlist)
		{
			grab->stop();
			delete (grab);
			grab = nullptr;

		}
		m_grabberlist.clear();
		
		// converter
		if (m_pconverter)
		{
			delete m_pconverter;
			m_pconverter = nullptr;
		}
		// gentl
		if (m_pgentl)
		{
			delete m_pgentl;
			m_pgentl = nullptr;
		}
		for (uint8_t* b : m_basebuf)
		{
			if (b != nullptr)
				free(b);
		}
		m_basebuf.fill(nullptr);

		if (m_buff)
		{
			free(m_buff);
			m_buff = nullptr;
		}
		if (nullptr != m_acqthread)
		{
			m_acqthread->join();
			delete m_acqthread;
			m_acqthread = nullptr;
		}
	}


	int Init(CamNfo& nfo);
	int Start();
	int Record();
	bool IsRecording();
	int Stop();
	int GetImage(UINT8 ** data);
	int GetImageInfo(ImgNfo& nfo);
	int GetStat(GrabStat& stat);
	int SetFps(double fps);
	int SetExposure(double exp);
	int GetExposure(double& exp);
	int SetResolution(size_t X, size_t Y);
	int GetRecordedRange(uint64_t& buffercount, uint64_t& start, uint64_t& end);
	int GetRecordImageAt(UINT8** data, uint64_t& at);
	int GetRecordedImageNext(UINT8** data, uint64_t& at);


};

