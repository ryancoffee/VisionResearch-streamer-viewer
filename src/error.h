#pragma once



#define SUCCESS					0
#define ERROR_NODRIVER			-100
#define ERROR_ACCESSDEVICE		-110
#define ERROR_NOINIT			-120
#define ERROR_NOSTARTED			-130
#define ERROR_NOMEMORY			-140
#define ERROR_PARAMOUTOFRANGE	-200
#define ERROR_PARAMACCESS		-210
#define ERROR_UNKNOWNDEV		-220
#define ERROR_UNKNOWNCAMERA		-230
#define ERROR_CAMERASETTING		-231
#define ERROR_START				-240
#define ERROR_STOP				-241
#define ERROR_ACQTIMEOUT		-500
#define ERROR_NOTIMPLEMENTED	-999




CString errorText(int errorCode);