#pragma once

#include "Extern.h"
#include "Synh_Objs.h"

// #include "CommonIP.h"
// using namespace IP_Camera;


class CCommonTimerThread
{
public:
	CCommonTimerThread(void);
	~CCommonTimerThread(void);

	// create common thread for grabbing cameras IP (see CameraIP)
	bool Init_TimerThread(void);

	// release all data
	void Release(void);

	// transfer state from outside
	void SetStateDevice(const LONG state_in);

	ListCamerasIP	*p_gList_Cameras_IP;

	CSynh_CritSect	m_SynhCS_Timer;

	CSynh_Barrier *m_pSynh_Bar_StCam_1;		// sinh. object barrier the state camera
	CSynh_Barrier *m_pSynh_Bar_StCam_2;		// sinh. object barrier the state camera

private:
	DWORD m_BaseTime_MSec;

	// Thread parameters
	HANDLE		m_hThread;
	DWORD			m_dwThreadId;

	//bool m_FlagCmnTimer_Thread;

	volatile LONG m_FlagCmnTimer_Thread;

	volatile LONG m_State_CmnTimer;

	//////////////////////////////////////////////////////////////////////////
	// threading function
	static DWORD Func_TimeTick_Thread(LPVOID lpParam);
};

