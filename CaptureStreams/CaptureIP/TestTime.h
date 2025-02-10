#pragma once

class CTestTime
{
public:

	CTestTime(void)
	{
		mseconds = 0;
		mcr_seconds = 0;
		li_Frequency.QuadPart = 1;	// to except zero divide
	}

	~CTestTime(void)
	{
	}
	
	// start high precision timer
 void StartTimer(void) {
	QueryPerformanceFrequency(&li_Frequency);   
 	QueryPerformanceCounter(&li_StartingTime);
 }

 //get millisecond
 ULONG CTestTime::EndTimer_MSec(void)
 {     
 	QueryPerformanceCounter(&li_EndingTime);      
 	li_EndingTime.QuadPart -= li_StartingTime.QuadPart;     
 	li_EndingTime.QuadPart *= 1000; // Adjust to milliseconds     
 	   
	mseconds = (ULONG)(li_EndingTime.QuadPart / li_Frequency.QuadPart);

 	return mseconds;
 }

 //get millisecond [double]
 double CTestTime::GetTimer_MSec(void)
 {     
	 QueryPerformanceCounter(&li_EndingTime);

	 return ((li_EndingTime.QuadPart - li_StartingTime.QuadPart)*1000.0 / li_Frequency.QuadPart);	//=== 1000; // Adjust to milliseconds ;
 }

 //get microsecond
 ULONG CTestTime::EndTimer_McrSec(void)	
 {     
	 QueryPerformanceCounter(&li_EndingTime);      
	 li_EndingTime.QuadPart -= li_StartingTime.QuadPart;     
	 li_EndingTime.QuadPart *= 1000000; // Adjust to microsecond

	 mcr_seconds = (ULONG)(li_EndingTime.QuadPart / li_Frequency.QuadPart);

	 return mcr_seconds;
 }

 ULONG Get_mSec(void)
 {
		return mseconds;
 }

 ULONG Get_mcrSec(void)
 {
	 return mcr_seconds;
 }

protected:
 ULONG mcr_seconds;
 ULONG mseconds;
 LARGE_INTEGER li_StartingTime;
 LARGE_INTEGER li_EndingTime;     
 LARGE_INTEGER li_Frequency;  

};

// class sleeping modulate (millisecond)
class CSleeping
{
public:
	CSleeping(void)
	{
	}
	~CSleeping(void)
	{
	}

	void Launch_Sleep(ULONG	time_ms_in)
	{
		m_TestTime.StartTimer();

		while(m_TestTime.GetTimer_MSec() <= time_ms_in){
			Sleep(1);
		}
	}

private:
	CTestTime	m_TestTime;
};
