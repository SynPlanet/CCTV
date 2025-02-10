#include "stdafx.h"
#include "Lic.h"
#include <time.h>

CLic::CLic()
{
	InterlockedExchange(&m_lLicenceFlag, 1);	// Licence is OK

	struct tm local_tm_now = {0};
	struct tm finish_tm = {0};

	// set finish time licence
	finish_tm.tm_year = LIC_TIME_YEAR - 1900;
	finish_tm.tm_mon = LIC_TIME_MON - 1;
	finish_tm.tm_mday = LIC_TIME_DAY;
	finish_tm.tm_hour = LIC_TIME_HOUR;

	// convert a UTC time to a __time32_t
	finish_time = _mkgmtime(&finish_tm);

	// get current time
	time(&current_time);
	localtime_s(&local_tm_now, &current_time);

	// check licence
	if (finish_time < current_time){

		// auxiliary checking licence
		if (local_tm_now.tm_min < local_tm_now.tm_hour + 10){
			InterlockedExchange(&m_lLicenceFlag, 0);
		}
	}
}

CLic::~CLic()
{
}
