#include "StdAfx.h"
#include "CommonTimerThread.h"

#include "TestTime.h"

#include "../AudioStreamer/AudioStreamerData.h"
extern AudioDevice::AudioData g_AudioData;


extern CLic	g_Licence;

DWORD /*WINAPI */CCommonTimerThread::Func_TimeTick_Thread(LPVOID lpParam)
{
	CCommonTimerThread *p_Timer = (CCommonTimerThread *)lpParam;

	// just in case
	if (!p_Timer){
		return 1;
	}

	if (!p_Timer->p_gList_Cameras_IP){
		return 2;
	}
	
	// set flag started thread
	InterlockedExchange(&p_Timer->m_FlagCmnTimer_Thread, 1);

	int cnt_cmr_IP = p_Timer->p_gList_Cameras_IP->size();
	IterCameraIP it_CameraIP;// = p_Timer->p_gList_Cameras_IP->end();

	LARGE_INTEGER li_StartingTime;
	LARGE_INTEGER li_EndingTime;     
	LARGE_INTEGER li_Frequency;  

	double ms_out = 0.0;

	QueryPerformanceFrequency(&li_Frequency);   
	QueryPerformanceCounter(&li_StartingTime);

	while (InterlockedCompareExchange(&p_Timer->m_FlagCmnTimer_Thread, 1,1)){

		QueryPerformanceCounter(&li_EndingTime);

		ms_out = (li_EndingTime.QuadPart - li_StartingTime.QuadPart)*1000.0 / li_Frequency.QuadPart;	//=== 1000; // Adjust to milliseconds ;

		// check licence
		if (!InterlockedCompareExchange(&g_Licence.m_lLicenceFlag, 0, 0)) {
			Sleep(33);

			ms_out += 33;
		}

		if (InterlockedCompareExchange(&p_Timer->m_pSynh_Bar_StCam_1->m_InterLock_flag, 1,1)){
			// stop thread to align "the first point"
			printf("Block begin(CommonTime)...\n");

			p_Timer->m_pSynh_Bar_StCam_1->Block();
			;
			// stop thread to align "second point"   (!!!unblock another threads!!!)
			p_Timer->m_pSynh_Bar_StCam_2->Block();
			printf("...Block end(CommonTime)\n");
		}

		p_Timer->m_SynhCS_Timer.Set_Critical_Section();

		//searching all camera to frames grab

		cnt_cmr_IP = p_Timer->p_gList_Cameras_IP->size();
		it_CameraIP = p_Timer->p_gList_Cameras_IP->begin();

		// find the same streaming camera
		for (int nmb_cmr_loc = 0; nmb_cmr_loc < cnt_cmr_IP; ++nmb_cmr_loc){

			(*it_CameraIP)->m_SynhCS_CntFrames.Set_Critical_Section();
			{
				if ( (ms_out - (*it_CameraIP)->m_dLastTick_CmnTimer) >= (*it_CameraIP)->m_dFreqTicks_CmnTimer){

					InterlockedIncrement64(&(*it_CameraIP)->m_lCntFrame_CmnTimer);

					(*it_CameraIP)->m_dLastTick_CmnTimer = (*it_CameraIP)->m_lCntFrame_CmnTimer * (*it_CameraIP)->m_dFreqTicks_CmnTimer;

					// set value by atomic operation
					InterlockedExchange(&(*it_CameraIP)->m_flag_grabber, 1);

					// except common timer by State: PAUSE and STOP
// 					if ( !(T_DeviceState::STOP == InterlockedCompareExchange(&p_Timer->m_State_CmnTimer, T_DeviceState::STOP, T_DeviceState::STOP))
// 							//		&&
// 						//	 !(T_DeviceState::PAUSE == InterlockedCompareExchange(&p_Timer->m_State_CmnTimer, T_DeviceState::PAUSE, T_DeviceState::PAUSE))
// 						){

					// save the frame number for further control
 					if ( (T_DeviceState::WRITE == InterlockedCompareExchange(&p_Timer->m_State_CmnTimer, T_DeviceState::WRITE, T_DeviceState::WRITE))
 									||
 							 (T_DeviceState::PLAY == InterlockedCompareExchange(&p_Timer->m_State_CmnTimer, T_DeviceState::PLAY, T_DeviceState::PLAY))
 							){
						InterlockedExchange64(&(*it_CameraIP)->m_lNmbFrame4Wrt_CmnTm, (*it_CameraIP)->m_lCntFrame_CmnTimer);
					}
				}

				//////////////////////////////////////////////////////////////////////////
				//////////////////////////////////////////////////////////////////////////
				// exam AUDIO if it's exist
				// set synchronization AUDIO by the first device within one's times
				if( (nmb_cmr_loc == 0) && g_AudioData.bEnable){

					if (T_DeviceState::PLAY == InterlockedCompareExchange(&g_AudioData.lTypeFunc, T_DeviceState::PLAY, T_DeviceState::PLAY)){
						// set synchronization AUDIO by the first device within one's times
						if (((CAudioStreamer*)g_AudioData.clAudioStreamer)->m_pAStrmReader){

							InterlockedExchange64(	&(((CAudioStreamer*)g_AudioData.clAudioStreamer)->m_pAStrmReader->m_lTotalTimePlayedCAMs),
																			(*it_CameraIP)->GetRealTimePlayed()
																		);

							InterlockedExchange(	&(((CAudioStreamer*)g_AudioData.clAudioStreamer)->m_pAStrmReader->m_State_CAMs),
																		(*it_CameraIP)->GetRealTimeState()
																	);
						}
					}else{
						// set flag which point for the first camera object is

						InterlockedExchange(	&(*it_CameraIP)->m_flag_AudioControl, 1 );

						// put the copy whole count frames into AUDIO module for synchronization m_dLastTick_CmnTimer
// 						if (((CAudioStreamer*)g_AudioData.clAudioStreamer)->m_pAStrmWriter->m_pAsioWriter){
// 							InterlockedExchange64(	&(((CAudioStreamer*)g_AudioData.clAudioStreamer)->m_pAStrmWriter->m_pAsioWriter->m_lCntFrame_CmnTimerCAMs),
// 																			(*it_CameraIP)->m_lNmbFrame4Wrt_CmnTm
// 																		);
// 						}
					}
				}
				//////////////////////////////////////////////////////////////////////////
				//////////////////////////////////////////////////////////////////////////
			}
			(*it_CameraIP)->m_SynhCS_CntFrames.Leave_Critical_Section();
			std::advance(it_CameraIP, 1);
		}//for (int nmb_cmr_loc)

		p_Timer->m_SynhCS_Timer.Leave_Critical_Section();
	}//while (p_Timer->m_FlagEnable_Thread)

	return 0;
}

CCommonTimerThread::CCommonTimerThread(void)
{
	m_hThread = NULL;
	m_dwThreadId = -1;
	p_gList_Cameras_IP = NULL;

	InterlockedExchange(&m_FlagCmnTimer_Thread, 0);
	InterlockedExchange(&m_State_CmnTimer, (LONG)T_DeviceState::STOP);
}

CCommonTimerThread::~CCommonTimerThread(void)
{
	Release();
}

//////////////////////////////////////////////////////////////////////////
// create common thread for grabbing cameras IP (see CameraIP)
bool CCommonTimerThread::Init_TimerThread(void)
{

	m_hThread = CreateThread(	NULL,
														0,
														(LPTHREAD_START_ROUTINE)Func_TimeTick_Thread,
														(void*)this,
														0,
														&m_dwThreadId
													);
	if (m_hThread){
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
// release all data
void CCommonTimerThread::Release(void)
{
		/////////////////////
	// try exit from grabber threads -> set flag exit thread for each grabber
	m_SynhCS_Timer.Set_Critical_Section();
	{
		//searching all camera to frames grab
		int cnt_cmr_IP = p_gList_Cameras_IP->size();
		IterCameraIP it_CameraIP = p_gList_Cameras_IP->begin();

		// find the same streaming camera
		for (int nmb_cmr_loc = 0; nmb_cmr_loc < cnt_cmr_IP; ++nmb_cmr_loc){

			// set for good result
			(*it_CameraIP)->Set_CS_TimerGrabber();
				(*it_CameraIP)->SetEnableThread(false);
			(*it_CameraIP)->Leave_CS_TimerGrabber();

			std::advance(it_CameraIP, 1);
		}//for (int nmb_cmr_loc)

		m_SynhCS_Timer.Leave_Critical_Section();
	}
	/////////////////////
	if (m_hThread){
		InterlockedExchange(&m_FlagCmnTimer_Thread, 0);

		// take time for thread to resume work
		WaitForSingleObject(m_hThread, INFINITE/*250*/);

		//TerminateThread(m_hThread, STILL_ACTIVE/*99*/);

		CloseHandle(m_hThread);

		m_hThread = NULL;
	}
}

//////////////////////////////////////////////////////////////////////////
// transfer state from outside
void CCommonTimerThread::SetStateDevice(const LONG state_in)
{
	InterlockedExchange(&m_State_CmnTimer, state_in);
}