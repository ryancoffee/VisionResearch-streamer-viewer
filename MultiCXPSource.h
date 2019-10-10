#pragma once
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
	
public :	
	MultiCXPSource()
	{
		// constructor
	}
	~MultiCXPSource()
	{
		// destructor
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

