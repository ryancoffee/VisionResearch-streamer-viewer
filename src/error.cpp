#include"pch.h"
#include "error.h"

CString errorText(int errorCode)
{

	CString code = L"Error unknown";
	switch (errorCode)
	{
	case SUCCESS: code = L"No error";
		break;
	case ERROR_NODRIVER: code = L"The driver for the frame grabber was not found";
		break;
	case ERROR_ACCESSDEVICE: code = L"Can not access frame grabber";
		break;
	case ERROR_NOINIT: code = L"System not properly initialized";
		break;
	case ERROR_NOSTARTED: code = L"System not properly started";
		break;
	case ERROR_NOMEMORY: code = L"System has not enough memory to proceed";
		break;
	case ERROR_PARAMOUTOFRANGE: code = L"Parameter out of range";
		break;
	case ERROR_PARAMACCESS: code = L"Parameter access denied";
		break;
	case ERROR_UNKNOWNDEV: code = L"Unknown device";
		break;
	case ERROR_ACQTIMEOUT: code = L"Acquisition timeout";
		break;
	case ERROR_NOTIMPLEMENTED: code = L"Feature not available";
		break;
	case ERROR_UNKNOWNCAMERA: code = L"Camera is not supported";
		break;
	case ERROR_CAMERASETTING: code = L"Failed to set camera configuration";
		break;
	case ERROR_START: code = L"Camera(s) could not be properly started";
		break;
	case ERROR_STOP: code = L"Camera(s) failed to stop";
		break;
	default:
		break;

	}
	return code;

}