#pragma once
class CControlHDD
{
public:
	CControlHDD(void);
	~CControlHDD(void);

	// create common thread for the control(examine) free space HDD
	bool Init_TimerThread(void);

	// release all data
	void Release(void);

private:
	DWORD m_BaseTime_MSec;

	//	CRITICAL_SECTION m_hCriticalSection;	// object for sync.classes CameraIP

	// Thread parameters
	HANDLE		m_hThread;
	DWORD			m_dwThreadId;

	// 
	volatile LONG m_FlagControl_Thread;	// the flag define a workability thread

	//////////////////////////////////////////////////////////////////////////
	// threading function
	static DWORD Func_ControlHDD_Thread(LPVOID lpParam);
};
