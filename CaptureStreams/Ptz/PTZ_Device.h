#pragma once

#include "../CaptureIP/Misc.h"



class CPTZ_Device
{
public:
	__declspec(dllexport) CPTZ_Device(void);
	__declspec(dllexport) virtual ~CPTZ_Device(void);

	__declspec(dllexport) int  Init_SDK_Samsung( void );
	__declspec(dllexport) int  Release_SDK_Samsung( void );

	__declspec(dllexport) int  Connect_Dev( T_NetAddress*	p_net_addr_in );
	__declspec(dllexport) int  Disconnect_Dev( T_NetAddress*	p_net_addr_in );

	__declspec(dllexport) int  Right( void );
	__declspec(dllexport) int  Left( void );
	__declspec(dllexport) int  Stop_Rotate( void );
	__declspec(dllexport) int  Up( void );
	__declspec(dllexport) int  Down( void );
	__declspec(dllexport) int  RightUp( void );
	__declspec(dllexport) int  RightDown( void );
	__declspec(dllexport) int  LeftUp( void );
	__declspec(dllexport) int  LeftDown( void );
	__declspec(dllexport) int  Speed( unsigned int speed_in ); // [0...100]
	__declspec(dllexport) int  ZoomIn( void );
	__declspec(dllexport) int  ZoomOut( void );
	__declspec(dllexport) int  Focus_Near( void );
	__declspec(dllexport) int  Focus_Far( void );
	__declspec(dllexport) int  Stop_Focus_Moving( void );

	__declspec(dllexport) int  GetPtzStatic( void );
	__declspec(dllexport) int  SetPtzStatic( int pan_in, int tilt_in, int zoom_in );
	__declspec(dllexport) void CallBackFunc_GetPtzValues(void* ptr_GetPtzValues);
};

