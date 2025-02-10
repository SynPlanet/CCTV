#include "StdAfx.h"
#include "NetPTZControl.h"
#include "TestTime.h"


extern Joy_IsConnect p_Joy_IsConnect;
extern Joy_Control	p_Joy_Control;
/**/

int g_Pan_PTZ_cur;	// current pan value (of PTZ)
int g_Tilt_PTZ_cur;	// current tilt value (of PTZ)
int g_Zoom_PTZ_cur;	// current zoom value (of PTZ)

//function for sending callback from CPTZ_Device
void __stdcall g_Callback_pGetPtzStatic(int pan_in, int tilt_in, int zoom_in)
{
	g_Pan_PTZ_cur = pan_in;
	g_Tilt_PTZ_cur = tilt_in;
	g_Zoom_PTZ_cur = zoom_in;
}

//////////////////////////////////////////////////////////////////////////
void GetMaxValuesPTZ(CNetPTZControl *p_NetPTZControl)
{
	if (!p_NetPTZControl){
		return;
	}

	DWORD startTime = GetTickCount();
	DWORD loc_Time = 0;
	const DWORD limit_sec = 3;

	int Pan_PTZ_Max = 0;	// current pan value (of PTZ)
	int Tilt_PTZ_Max = 0;	// current tilt value (of PTZ)
	int Zoom_PTZ_Max = 0;	// current zoom value (of PTZ)

	while((GetTickCount() - startTime) /1000 < limit_sec){
			// send flags for getting MAXIMUM values PanTiltZoom
		p_NetPTZControl->SetPtzStatic(-1, -1, -1);

		// set flag getting parameters from PTZ camera by CallbackFunction
		p_NetPTZControl->GetPtzStatic();

		if (g_Pan_PTZ_cur > Pan_PTZ_Max){
			Pan_PTZ_Max = g_Pan_PTZ_cur;
		}
		if (g_Tilt_PTZ_cur > Tilt_PTZ_Max){
			Tilt_PTZ_Max = g_Tilt_PTZ_cur;
		}
		if (g_Zoom_PTZ_cur > Zoom_PTZ_Max){
			Zoom_PTZ_Max = g_Zoom_PTZ_cur;
		}

	}
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
DWORD /*WINAPI*/ CNetPTZControl::PTZ_Joy_Data_Thread(LPVOID lpParam)
{
	CNetPTZControl *p_NetPTZControl = (CNetPTZControl *)lpParam;

	unsigned int state_controle_old = 0;
	unsigned int _state_controle_old = -1;
	unsigned int _cur_cam_old = -1;

	// just in case
	if (!p_NetPTZControl){
		return 1;
	}
	if (!p_NetPTZControl->m_hEvent_SendCmdPTZ){
		return 1;
	}

	// set callback function
	//p_NetPTZControl->CallBackFunc_GetPtzValues((void*)/*_Callback_pGetPtzStatic*/ NULL);

	bool state_joy_old = false;
	DWORD dwWaitResult = WAIT_FAILED;

	// set value by atomic operation
	InterlockedExchange(&p_NetPTZControl->m_flag_Enable_Loop, 1);

	//flag define net connecting possibility and send/get data to/from receiver
	bool	bFlagNetSend = false;

	// first initialization
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	//send data to device PTZ!!!
//	bool flag_get_camera_max_values = false;
// 	if (!flag_get_camera_max_values){
// 		// get maximum values PTZ
// 		//GetMaxValuesPTZ(p_NetPTZControl);
// 
// 		// set Camera to base coordinates
// 		p_NetPTZControl->SetPtzStatic(0, 0, 0);
// 
// 		flag_get_camera_max_values = true;
// 	}
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	while (	InterlockedCompareExchange(&p_NetPTZControl->m_flag_Enable_Loop, 1,1)	)
	{
		////////////////////////////////////////
// 		if (p_NetPTZControl->m_flag_SetNmbCmr){
// 
// 			p_NetPTZControl->m_JoyData.cur_cam = p_NetPTZControl->m_NmbCam_3D;
// 
// 			p_NetPTZControl->m_flag_SetNmbCmr = false;
// 		}
		////////////////////////////////////////
		// handling joystick
		if (p_NetPTZControl->IsJoyEnable()){

			p_NetPTZControl->Retrieve_Joy_Data(); // get data for structure T_DataPTZ

			if (p_Joy_Control){
				if ( (_state_controle_old != p_NetPTZControl->m_JoyData.state_controle)
							||
							( _cur_cam_old != p_NetPTZControl->m_JoyData.cur_cam)
					){
					// send data for graphics interface(output)
					p_Joy_Control(p_NetPTZControl->m_JoyData.cur_cam, p_NetPTZControl->m_JoyData.state_controle);

					if (_state_controle_old != p_NetPTZControl->m_JoyData.state_controle){
						SetEvent(p_NetPTZControl->m_hEvent_SendCmdPTZ);

						_state_controle_old = p_NetPTZControl->m_JoyData.state_controle;
					}
					
					_cur_cam_old = p_NetPTZControl->m_JoyData.cur_cam;
				}
			}
		}

		////////////////////////////////////////
		// synh. object (set sleep 33 Hz)
		dwWaitResult = WaitForSingleObject( p_NetPTZControl->m_hEvent_SendCmdPTZ, 30/*INFINITE*/ );

		switch (dwWaitResult) 
		{
			// Event object was signaled
		case WAIT_OBJECT_0: 
			{
				// try to joining
				if (InterlockedCompareExchange(&p_NetPTZControl->m_flag_SetNewCmr, 1,1) ){

					//p_NetPTZControl->Disconnect_Dev(NULL); // "Disconnect_Dev" see "Connect_Dev"

					if(p_NetPTZControl->m_pCameraIP_Joined){	// just in case

						// try to join to camera
						if(p_NetPTZControl->Connect_Dev((T_NetAddress *)p_NetPTZControl->m_pCameraIP_Joined->GetInfoAddress()) >= 0){	//	if(p_NetPTZControl->Connect_Dev(&p_NetPTZControl->m_Cmr_Join) >= 0){
							bFlagNetSend = true;
						}
					}

					/// search current camera for the control
					if(p_NetPTZControl->m_pCameraIP_Joined){
						// was chosen need camera 
						if (p_NetPTZControl->m_JoyData.cur_cam != p_NetPTZControl->m_pCameraIP_Joined->GetNmbCameraIP()){

							p_NetPTZControl->Stop_Rotate();						// [12.02.15]
							p_NetPTZControl->Stop_Focus_Moving();			// [12.02.15]

							ResetEvent(p_NetPTZControl->m_hEvent_SendCmdPTZ);// rest output event 
							continue;
						}
					}else{

						ResetEvent(p_NetPTZControl->m_hEvent_SendCmdPTZ);// rest output event 
						continue;
					}
					///

					// reset flag
					InterlockedExchange(&p_NetPTZControl->m_flag_SetNewCmr, 0);
				}

				//////////////////////////////////////
				//send data to device PTZ!!!
				if (bFlagNetSend){
					//printf("state_controle:%d", p_NetPTZControl->m_JoyData.state_controle);

					// set camera default
					if (	p_NetPTZControl->m_JoyData.state_controle & T_ControlPTZ::BASE_POS	){
						p_NetPTZControl->SetPtzStatic(0, 0, 0);

						ResetEvent(p_NetPTZControl->m_hEvent_SendCmdPTZ);// rest output event 
						continue;
					}

					// set flag getting parameters from PTZ camera by CallbackFunction
				//	p_NetPTZControl->GetPtzStatic();
					int flag_stop_move = false;
					if( !(	(p_NetPTZControl->m_JoyData.state_controle & T_ControlPTZ::DOWN)	|| 
									(p_NetPTZControl->m_JoyData.state_controle & T_ControlPTZ::UP)		||
									(p_NetPTZControl->m_JoyData.state_controle & T_ControlPTZ::RIGHT)	||
									(p_NetPTZControl->m_JoyData.state_controle & T_ControlPTZ::LEFT)	||
									(p_NetPTZControl->m_JoyData.state_controle & T_ControlPTZ::ZOOM_IN)	||
									(p_NetPTZControl->m_JoyData.state_controle & T_ControlPTZ::ZOOM_OUT)
								)
						){
						p_NetPTZControl->Stop_Rotate();

						flag_stop_move = true;
					}else{
						// see structure T_DataPTZ
						if (p_NetPTZControl->m_JoyData.state_controle & T_ControlPTZ::FORSAGE){
							p_NetPTZControl->Speed(100);
						}else{
							if (p_NetPTZControl->m_JoyData.state_controle & T_ControlPTZ::SPEED_HIGH){
								p_NetPTZControl->Speed(60);
							}else{
								if (p_NetPTZControl->m_JoyData.state_controle & T_ControlPTZ::SPEED_LOW){
									p_NetPTZControl->Speed(p_NetPTZControl->m_JoyData.speed_moving);
								}else{
									p_NetPTZControl->Speed(0);
								}
							}
						}

						if(p_NetPTZControl->m_JoyData.state_controle & T_ControlPTZ::DOWN){
							p_NetPTZControl->Down();
						}else{
							if(p_NetPTZControl->m_JoyData.state_controle & T_ControlPTZ::UP){
								p_NetPTZControl->Up();
							}
						}

						if(p_NetPTZControl->m_JoyData.state_controle & T_ControlPTZ::RIGHT){
							p_NetPTZControl->Right();
						}else{
							if(p_NetPTZControl->m_JoyData.state_controle & T_ControlPTZ::LEFT){
								p_NetPTZControl->Left();
							}
						}

						if(p_NetPTZControl->m_JoyData.state_controle & T_ControlPTZ::ZOOM_IN){
							p_NetPTZControl->ZoomIn();
						}else{
							if(p_NetPTZControl->m_JoyData.state_controle & T_ControlPTZ::ZOOM_OUT){
								p_NetPTZControl->ZoomOut();
							}
						}
						///////////////////////////////////////
						///////////////////////////////////////
						/*
						// controlling "STOP" command by VIRTUAL joystick
						if (p_NetPTZControl->m_JoyData.dev_type == T_JoyDevice::VIRTUAL){	
						}
						*/
						///////////////////////////////////////
						///////////////////////////////////////
					}

					if( !(	(p_NetPTZControl->m_JoyData.state_controle & T_ControlPTZ::FOCUS_FAR)	|| 
									(p_NetPTZControl->m_JoyData.state_controle & T_ControlPTZ::FOCUS_NEAR)
								)
								&&
								!flag_stop_move
						){
							p_NetPTZControl->Stop_Focus_Moving();
					}else{
						if (p_NetPTZControl->m_JoyData.state_controle & T_ControlPTZ::FOCUS_NEAR){
							p_NetPTZControl->Focus_Near();
						}else{
							if (p_NetPTZControl->m_JoyData.state_controle & T_ControlPTZ::FOCUS_FAR){
								p_NetPTZControl->Focus_Far();
							}
						}
					}

					state_controle_old = p_NetPTZControl->m_JoyData.state_controle;

				}
				// rest output event 
				ResetEvent(p_NetPTZControl->m_hEvent_SendCmdPTZ);
				//
			}

			break; 

			// time is out
		default: 
			{
				//check joystick connect
				if ( (p_NetPTZControl->IsJoyEnable() != state_joy_old) && p_Joy_IsConnect){
					state_joy_old = p_NetPTZControl->IsJoyEnable();

					p_Joy_IsConnect(state_joy_old);
				}
			};
		} // switch (dwWaitResult)
		//////////////////////////////////////

	}//while ()

	// final command -> STOP all moves
	p_NetPTZControl->Stop_Rotate();
	p_NetPTZControl->Stop_Focus_Moving();	

  return 0;
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//function for sending callback from CPTZ_Device
// void CNetPTZControl::Callback_pGetPtzStatic(int pan_in, int tilt_in, int zoom_in)
// {
// 	m_Pan_PTZ_cur = pan_in;
// 	m_Tilt_PTZ_cur = tilt_in;
// 	m_Zoom_PTZ_cur = zoom_in;
// 	m_FlagGetValPTZ = true;
//}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

CNetPTZControl::CNetPTZControl(void)
{
	// set default
	InterlockedExchange(&m_flag_Enable_Loop, 0);
	InterlockedExchange(&m_flag_SetNewCmr, 0);

	m_hNetThread = NULL;
	m_dwNetThreadId = NULL;

// 	m_FlagGetValPTZ = false;
// 	m_Pan_PTZ_cur = 0;
// 	m_Tilt_PTZ_cur = 0;
// 	m_Zoom_PTZ_cur = 0;
//	m_NmbCam_3D = -1;

	//m_flag_SetNmbCmr = false;

	m_pCameraIP_Joined = NULL;

	m_hEvent_SendCmdPTZ = NULL;

	ZeroMemory((void*)&m_Cmr_Join, sizeof(m_Cmr_Join));
}


CNetPTZControl::~CNetPTZControl(void)
{
//	Release();
}

// create net sending using thread
int CNetPTZControl::Init( void )
{
	// initialization joystick
	Init_Joystick();

	//initialization SDK Samsung
	printf("Init_SDK_Samsung:...\n");
	printf(">>\n");
		Init_SDK_Samsung();
	printf("<<\n");

	// create event for send PTZ command 
	m_hEvent_SendCmdPTZ = CreateEvent( NULL, TRUE, FALSE, NULL );

	if (m_hEvent_SendCmdPTZ){
		
		m_hNetThread = CreateThread(	NULL,
																	0,
																	(LPTHREAD_START_ROUTINE)PTZ_Joy_Data_Thread, 
																	(void*)this,
																	0,
																	&m_dwNetThreadId
																);
		if (m_hNetThread && m_hEvent_SendCmdPTZ){
			// set accuracy processor for the created thread
			//DWORD err_ = SetThreadIdealProcessor(m_hGrabThread, m_nmb_Processor);

			// send pointer function for getting parameters PTZ
			//CallBackFunc_GetPtzValues((void*)&Callback_pGetPtzStatic);

			CallBackFunc_GetPtzValues((void*)&g_Callback_pGetPtzStatic);

			BOOL err = SetThreadPriority(m_hNetThread, THREAD_PRIORITY_LOWEST);

			printf("PTZ_enable: TRUE!\n");

			return 0;
		}else{
			return -1;
		}
	}

	return -2;
}

//////////////////////////////////////////////////////////////////////////
// get state the thread of PTZ class
bool CNetPTZControl::IsEnable(void)
{
	return ((bool)m_flag_Enable_Loop);
}

//////////////////////////////////////////////////////////////////////////
int CNetPTZControl::Release(void)
{
	// terminate thread
	if (m_hNetThread){
		// set value by atomic operation
		InterlockedExchange(&m_flag_Enable_Loop, 0);

		// take time for thread to resume work
		WaitForSingleObject(m_hNetThread, INFINITE/*250*/);

		//TerminateThread(m_hNetThread, NULL/*STILL_ACTIVE*/);
		CloseHandle(m_hNetThread);

		m_hNetThread = NULL;
	}

	if (m_hEvent_SendCmdPTZ){
		// rest output event 
		ResetEvent(m_hEvent_SendCmdPTZ);

		CloseHandle(m_hEvent_SendCmdPTZ);
		m_hEvent_SendCmdPTZ = NULL;
	}

	// Release any DirectInput objects
	printf("Joystick->Release...!\n");
	printf(">>>>\n");
	{
		ReleaseJoystick();
	}
	printf("<<<<\n");

	// release SDK samsung
	printf("SDK_Samsung->Release...!\n");
	printf(">>>>\n");
	{
		Release_SDK_Samsung();
	}
	printf("<<<<\n");

	return -1;
}

//////////////////////////////////////////////////////////////////////////
// data was gotten from output interface (real or virtual) for camera control
void CNetPTZControl::SetJoyControl( const LONG nmb_Cmr_in,
																		const LONG state_in
																	)
{
	// just in case
	if (!IsJoyEnable()){
		InterlockedExchange(&m_JoyData.cur_cam, nmb_Cmr_in);
		InterlockedExchange(&m_JoyData.state_controle, state_in);
	}

	if (m_JoyData.dev_type == T_JoyDevice::VIRTUAL){
		;
	}

	// set event for one working thread 
	if (m_hEvent_SendCmdPTZ){
		SetEvent(m_hEvent_SendCmdPTZ);
	}
}

//////////////////////////////////////////////////////////////////////////
// get flag enable 360 of camera
bool CNetPTZControl::GetFlag360( T_NetAddress	*p_net_adr_in )
{
	if (!p_net_adr_in){
		return false;
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////
// set number camera 
unsigned int CNetPTZControl::SetNmbCam( const unsigned int nmb_Cam_3D )
{
	InterlockedExchange(&m_JoyData.cur_cam, (LONG)nmb_Cam_3D);

	//m_NmbCam_3D = nmb_Cam_3D;
	//m_flag_SetNmbCmr = true;

	// set event for one working thread 
	if (m_hEvent_SendCmdPTZ){
		SetEvent(m_hEvent_SendCmdPTZ);
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
// join to new camera for control
int CNetPTZControl::SetCameraPTZ_Control( const CCameraIP*	p_CameraIP_in)
{
	if (p_CameraIP_in){
		if (p_CameraIP_in->GetFlag360()){

			if(m_JoyData.cur_cam == ((CCameraIP*)p_CameraIP_in)->GetNmbCameraIP()){

				// save current camera for controlling 
				m_pCameraIP_Joined = (CCameraIP*)p_CameraIP_in;

				InterlockedExchange(&m_flag_SetNewCmr, 1);

				// set event for one working thread 
				if (m_hEvent_SendCmdPTZ){
					SetEvent(m_hEvent_SendCmdPTZ);
				}
			}

		}else{
			;// it is not camera 3D
			return -1;
		}
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////
// set event to resume thread the working for the single step
void CNetPTZControl::RunSynhThread(void)
{
	if (m_hEvent_SendCmdPTZ){
		SetEvent(m_hEvent_SendCmdPTZ);
	}
}