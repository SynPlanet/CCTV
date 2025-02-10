#pragma once

#include <winsock2.h>
#include "CommonIP.h"
#include "JoyControl.h"
#include "CameraIP.h"
#include "../Ptz/PTZ_Device.h"

// the class defined control PanTiltZoom for camera 3D
class CNetPTZControl: public CJoyControl, public CPTZ_Device
{
public:
	CNetPTZControl(void);
	~CNetPTZControl(void);

	//CJoyControl	m_JoyControl;

	int Init( void );

	int Release(void);

	bool IsEnable(void);

	// threading function for handling joystick and send data to PTZ
	static DWORD PTZ_Joy_Data_Thread(LPVOID lpParam);

	// data was gotten from output interface (real or virtual) for camera control
	void SetJoyControl( const LONG nmb_Cmr_in,
											const LONG state_in
										);

	// join to new camera for control
	//int SetCameraPTZ_Control( const T_NetAddress	*p_net_adr_in, const UINT nmb_cam_3D);

	// join to new camera for control
	int SetCameraPTZ_Control( const CCameraIP*	p_CameraIP_in);
	// get flag enable 360 of camera
	bool GetFlag360( T_NetAddress	*p_net_adr_in );

	//function for sending callback from CPTZ_Device
	//static __declspec(dllexport) void __stdcall Callback_pGetPtzStatic(int pan_in, int tilt_in, int zoom_in);

// public:
// 	int m_Pan_PTZ_cur;	// current pan value (of PTZ)
// 	int m_Tilt_PTZ_cur;	// current tilt value (of PTZ)
// 	int m_Zoom_PTZ_cur;	// current zoom value (of PTZ)
// 	bool m_FlagGetValPTZ;	// flag define PTZ values had already gotten

	// set number camera 
	unsigned int SetNmbCam( const unsigned int nmb_Cam_3D );

	// get number camera 
	const unsigned int GetNmbCam( void ){return m_JoyData.cur_cam;}

	// set event to resume thread the working for the single step
	void RunSynhThread(void);

private:
	volatile LONG m_flag_Enable_Loop;	// the flag define enable loop thread
	volatile LONG m_flag_SetNewCmr; // flag defined joining of a new camera

	HANDLE	m_hNetThread;
	DWORD   m_dwNetThreadId;

	T_NetAddress m_Cmr_Join;	// camera was joined for handling
//	unsigned int m_NmbCam_3D;	// number 3D camera was connected

	CCameraIP*	m_pCameraIP_Joined;	// pointer 

	//bool m_flag_SetNmbCmr; // flag defined number camera for handling

	// object for event for send PTZ command 
	HANDLE m_hEvent_SendCmdPTZ;

};

