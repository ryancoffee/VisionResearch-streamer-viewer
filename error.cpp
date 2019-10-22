#include"pch.h"
#include "error.h"

CString errorText(int errorCode)
{

	CString code = L"Error unknown";
	switch (errorCode)
	{
	case 0: code = L"No error";
		break;
	case -100: code = L"The driver for the frame grabber was not found";
		break;
	case -110: code = L"Can not access frame grabber";
		break;
	case -120: code = L"System not properly initialized";
		break;
	case -130: code = L"System not properly started";
		break;
	case -200: code = L"Parameter out of range";
		break;
	case -210: code = L"Parameter access denied";
		break;
	case -220: code = L"Unknown device";
		break;
	case -500: code = L"Acquisition timeout";
		break;
	default:
		break;

	}
	return code;

}