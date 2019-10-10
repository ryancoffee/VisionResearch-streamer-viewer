#include "pch.h"
#include "MultiCXPSource.h"

int MultiCXPSource::Init(CamNfo& nfo)
{
	nfo.name = L" S640";
	nfo.lnkCount = 4; 
	return 0;
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
