#pragma once

//////////////////////////////////////////////////////////////////////////
//https://msdn.microsoft.com/ru-ru/magazine/cc163427.aspx#S3
// class Barrier
// define sinh. method among several thread
class CSynh_Barrier
{
public:

	CSynh_Barrier(void):
			m_FlagEnable(false),
				m_sense(false),
				m_InterLock_flag(0),
				m_uiWaitTime(5000)
			{
				m_count = 1;
				m_originalCount = 1;

				m_hOddEvent=CreateEvent(	NULL,               // default security attributes
																	TRUE,               // manual-reset event
																	FALSE,              // initial state is nonsignaled
																	NULL	// object name
																	);
				m_hEvenEvent=CreateEvent(	NULL,               // default security attributes
																	TRUE,               // manual-reset event
																	FALSE,              // initial state is nonsignaled
																	NULL	// object name
																);
				m_FlagEnable = true;
			}

			~CSynh_Barrier(void)
			{
				// delete sync.object
				if (m_hOddEvent){
					CloseHandle(m_hOddEvent);
					m_hOddEvent = NULL;
				}
				if (m_hEvenEvent){
					CloseHandle(m_hEvenEvent);
					m_hEvenEvent = NULL;
				}				

				m_FlagEnable = false;
			}

			//////////////////////////////////////////////////////////////////////////
			// set number objects for barrier using
			void SetNmbBarrier(int count)
			{
				m_count = count;
				m_originalCount = count;
			}

			//////////////////////////////////////////////////////////////////////////
			// the main method block thread for sinhronization
			const BOOL Block(void)
			{
				bool sense = m_sense;

				// The last thread to signal also sets the event.
				if ( (m_count == 1)	||	(InterlockedDecrement(&m_count) == 0 )
					){

						m_count = m_originalCount;
						m_sense = !sense; // Reverse the sense.
						if (sense == true){
							// odd
							ResetEvent(m_hEvenEvent);
							SetEvent(m_hOddEvent);
						}else{
							ResetEvent(m_hOddEvent);
							SetEvent(m_hEvenEvent);
						}
				}else{
					if (sense == true){
						if(WaitForSingleObject(m_hOddEvent, INFINITE)){
							// max value counter overriding
							;//error
						//	m_originalCount--;
							return FALSE;
						}
					}else{
						if(WaitForSingleObject(m_hEvenEvent, INFINITE)){
							// max value counter overriding
							;//error
						//	m_originalCount--;
							return FALSE;
						}
					}
				}
				return TRUE;
			}

			volatile LONG m_InterLock_flag;	// value for using by Interloc...

private:
	volatile LONG m_count;
	int m_originalCount;
	HANDLE m_hOddEvent;
	HANDLE m_hEvenEvent;

	volatile bool m_sense; // false==even, true==odd.

	unsigned int m_uiWaitTime;	// wait time sleeping (see WaitForSingleObject() ) [msec]

	bool m_FlagEnable;	// flag define enable object critical section
};
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// one of synh. class 
class CSynh_CritSect
{
public:

	CSynh_CritSect(void):
			m_FlagEnable(false),
			m_synq_flag(0)
	{
		ZeroMemory((void*) &m_h_CriticalSection, sizeof(m_h_CriticalSection));

		InitializeCriticalSection( &m_h_CriticalSection );

		m_hEvent=CreateEvent(	NULL,               // default security attributes
													TRUE,               // manual-reset event
													FALSE,              // initial state is nonsignaled
													NULL	// object name
												);

		m_FlagEnable = true;
	}

	~CSynh_CritSect(void)
	{
		// delete sync.object
		CloseHandle(m_hEvent);

		DeleteCriticalSection(&m_h_CriticalSection);
		m_FlagEnable = false;
	}

	// set critical section in working state
	void Set_Critical_Section(void)
	{
		if (m_FlagEnable){
			EnterCriticalSection( &m_h_CriticalSection );
		}
	};

	// try to set critical section in working state
	const BOOL TrySet_Critical_Section(void)
	{
		if (m_FlagEnable){
			return TryEnterCriticalSection ( &m_h_CriticalSection );
		}
		return FALSE;
	};

	// set working out critical section 
	void Leave_Critical_Section(void)
	{
		if (m_FlagEnable){
			LeaveCriticalSection( &m_h_CriticalSection );
		}
	};

	// try to set critical section in working state
	const LONG SetBarrier(void)
	{
		return InterlockedExchange(&m_synq_flag, 1);
		//SetEvent(m_hEvent);
	};

	// try to set critical section in working state
	const BOOL LeaveBarrier(void)
	{
		InterlockedExchange(&m_synq_flag, 0);
		return SetEvent(m_hEvent);
	};

	volatile LONG m_synq_flag;	// value for using by Interloc...
	HANDLE m_hEvent;

private:

	bool m_FlagEnable;	// flag define enable object critical section

	CRITICAL_SECTION m_h_CriticalSection;	// object for sync.classes CameraIP
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// class "Single Writer - Multiple Readers"
// Jeffrey Richter 4 Edition Windows Programming Applications for Microsoft 2008
class CSync_SWMR
{
public:
	CSync_SWMR() {
		// Initially no readers want access, no writers want access, and no threads are accessing the resource
		m_nWaitingReaders = m_nWaitingWriters = m_nActive = 0;
		m_hsemReaders = CreateSemaphore(NULL, 0, MAXLONG, NULL);
		m_hsemWriters = CreateSemaphore(NULL, 0, MAXLONG, NULL);
		InitializeCriticalSection(&m_cs);
	}
	~CSync_SWMR() {
		#ifdef _DEBUG
				// A SWMRG shouldn't be destroyed if any threads are using the resource
				if (m_nActive != 0)
					DebugBreak();
		#endif
				m_nWaitingReaders = m_nWaitingWriters = m_nActive = 0;
				DeleteCriticalSection(&m_cs);
				CloseHandle(m_hsemReaders);
				CloseHandle(m_hsemWriters);
	}

	// Call this to gain shared read access
	VOID WaitToRead(VOID) {
		// Ensure exclusive access to the member variables
		EnterCriticalSection(&m_cs);
		// Are there writers waiting or is a writer writing?
		BOOL fResourceWritePending = (m_nWaitingWriters || (m_nActive < 0));
		if (fResourceWritePending) {
			// This reader must wait, increment the count of waiting readers
			m_nWaitingReaders++;
		} else {
			// This reader can read, increment the count of active readers
			m_nActive++;
		}
		// Allow other threads to attempt reading/writing
		LeaveCriticalSection(&m_cs);
		if (fResourceWritePending) {
			// This thread must wait
			WaitForSingleObject(m_hsemReaders, INFINITE);
		}
	}
	// Call this to gain exclusive write access
	VOID WaitToWrite(VOID) {
		// Ensure exclusive access to the member variables
		EnterCriticalSection(&m_cs);
		// Are there any threads accessing the resource?
		BOOL fResourceOwned = (m_nActive != 0);
		if (fResourceOwned) {
			// This writer must wait, increment the count of waiting writers
			m_nWaitingWriters++;
		} else {
			// This writer can write, decrement the count of active writers
			m_nActive = -1;
		}
		// Allow other threads to attempt reading/writing
		LeaveCriticalSection(&m_cs);
		if (fResourceOwned) {
			// This thread must wait
			WaitForSingleObject(m_hsemWriters, INFINITE);
		}
	}

	// Call this when done accessing the resource
	VOID Done(VOID) {
		// Ensure exclusive access to the member variables
		EnterCriticalSection(&m_cs);
		if (m_nActive > 0) {
			// Readers have control so a reader must be done
			m_nActive--;
		} else {
			// Writers have control so a writer must be done
			m_nActive++;
		}
		HANDLE hsem = NULL; // Assume no threads are waiting
		LONG lCount = 1; // Assume only 1 waiter wakes; always true for writers
		if (m_nActive == 0) {
			// No thread has access, who should wake up?
			// NOTE: It is possible that readers could never get access if there are always writers wanting to write
			if (m_nWaitingWriters > 0) {
				// Writers are waiting and they take priority over readers
				m_nActive = -1; // A writer will get access
				m_nWaitingWriters--; // One less writer will be waiting
				hsem = m_hsemWriters; // Writers wait on this semaphore
				// NOTE: The semaphore will release only 1 writer thread
			} else if (m_nWaitingReaders > 0) {
				// Readers are waiting and no writers are waiting
				m_nActive = m_nWaitingReaders; // All readers will get access
				m_nWaitingReaders = 0; // No readers will be waiting
				hsem = m_hsemReaders; // Readers wait on this semaphore
				lCount = m_nActive; // Semaphore releases all readers
			} else {
				// There are no threads waiting at all; no semaphore gets released
			}
		}
		// Allow other threads to attempt reading/writing
		LeaveCriticalSection(&m_cs);
		if (hsem != NULL) {
			// Some threads are to be released
			ReleaseSemaphore(hsem, lCount, NULL);
		}
	}

private:
	CRITICAL_SECTION m_cs; // Permits exclusive access to other members
	HANDLE m_hsemReaders; // Readers wait on this if a writer has access
	HANDLE m_hsemWriters; // Writers wait on this if a reader has access
	int m_nWaitingReaders; // Number of readers waiting for access
	int m_nWaitingWriters; // Number of writers waiting for access
	int m_nActive; // Number of threads currently with access
	// (0=no threads, >0=# of readers, -1=1 writer)

};
