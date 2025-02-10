#include "StdAfx.h"
#include "ControlHDD.h"

//https://support.microsoft.com/en-us/kb/231497


#include "CommonIP.h"
#include "CaptureIP.h"

using namespace IP_Camera;

extern const int UpdataStateStream(	const UINT nmb_Cmr,		// number of current camera [-1 ... N]
																		const UINT new_state	// new state for further control audio-video streaming(see T_DeviceState)
																	);

CControlHDD::CControlHDD(void)
{
	InterlockedExchange(&m_FlagControl_Thread, 0);
}

CControlHDD::~CControlHDD(void)
{
	Release();
}

//////////////////////////////////////////////////////////////////////////
// create common thread for the control(examine) free space HDD
bool CControlHDD::Init_TimerThread(void)
{
	bool err = false;
	InterlockedExchange(&m_FlagControl_Thread, 1);

	m_hThread = CreateThread(	NULL,
														0,
														(LPTHREAD_START_ROUTINE)Func_ControlHDD_Thread,
														(void*)this,
														0,
														&m_dwThreadId
													);

	if (m_hThread){

		err = SetThreadPriority(m_hThread, THREAD_PRIORITY_LOWEST);
	}else{
		InterlockedExchange(&m_FlagControl_Thread, 0);
	}

	return m_FlagControl_Thread;
}

//////////////////////////////////////////////////////////////////////////
// release all data
void CControlHDD::Release(void)
{
	if (m_hThread){
		InterlockedExchange(&m_FlagControl_Thread, 0);

		// take time for thread to resume work
		WaitForSingleObject(m_hThread, 500);

		TerminateThread(m_hThread, STILL_ACTIVE/*99*/);

		CloseHandle(m_hThread);

		m_hThread = NULL;
	}
}

DWORD /*WINAPI */CControlHDD::Func_ControlHDD_Thread(LPVOID lpParam)
{
	CControlHDD *p_ControlHDD = (CControlHDD *)lpParam;

	// just in case
	if (!p_ControlHDD){
		return 1;
	}

	char  *pszDrive  = NULL,
				szDrive[4];

	DWORD dwSectPerClust,
				dwBytesPerSect,
				dwFreeClusters,
				dwTotalClusters;

	unsigned __int64	i64FreeBytesToCaller,
										i64TotalBytes,
										i64FreeBytes;
	unsigned __int64	sz_free_HDD = 0;

	BOOL  fResult;

	fResult = GetDiskFreeSpaceEx (	L"C:\\",
																(PULARGE_INTEGER)&i64FreeBytesToCaller,
																(PULARGE_INTEGER)&i64TotalBytes,
																(PULARGE_INTEGER)&i64FreeBytes
															);

	printf ("\n\n");
	printf ("Available space to caller = %I64u MB\n",				i64FreeBytesToCaller / (1024*1024));
	printf ("Total space               = %I64u MB\n",				i64TotalBytes / (1024*1024));
	//printf ("Free space on drive       = %I64u MB\n",				sz_free_HDD);
	printf ("\n");

	while (InterlockedCompareExchange(&p_ControlHDD->m_FlagControl_Thread, 1,1)){

		fResult = GetDiskFreeSpaceEx (	L"C:\\",
																		(PULARGE_INTEGER)&i64FreeBytesToCaller,
																		(PULARGE_INTEGER)&i64TotalBytes,
																		(PULARGE_INTEGER)&i64FreeBytes
																	);

		if (fResult)
		{
			sz_free_HDD = i64FreeBytes / (1024*1024);

			// compare require size HDD for grabbing
			if (sz_free_HDD < SIZE_HDD_STOP_GRAB_VIDEO_MB){

				// get state the first camera
				long state_stream_cmr = GetStateCamera(0);

				if (state_stream_cmr == (LONG)T_DeviceState::WRITE){

					printf ("\n\n");
					printf ("Available space to caller = %I64u MB\n",				i64FreeBytesToCaller / (1024*1024));
					printf ("Total space               = %I64u MB\n",				i64TotalBytes / (1024*1024));
					//printf ("Free space on drive       = %I64u MB\n",				sz_free_HDD);
					printf ("\n");

					// stop grabbing all devices => HDD memory had been full
					UpdataStateStream(	-1,												// number of current camera
															(LONG)T_DeviceState::STOP	// the flag of writing the video streaming from Camera
														);
				}
			}
		}

		//sleep needed time
		Sleep(TIME_CONTROL_HDD_MSEC);
	}

	return 0;
}