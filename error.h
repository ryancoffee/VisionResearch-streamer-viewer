#pragma once



#define SUCCESS					0
#define ERROR_NODRIVER			-100
#define ERROR_ACCESSDEVICE		-110
#define ERROR_NOINIT			-120
#define ERROR_NOSTARTED			-130
#define ERROR_PARAMOUTOFRANGE	-200
#define ERROR_PARAMACCESS		-210
#define ERROR_UNKNOWNDEV		-220
#define ERROR_ACQTIMEOUT		-500




CString errorText(int errorCode);
