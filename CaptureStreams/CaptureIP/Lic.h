#pragma once

#define LIC_TIME_YEAR	2020
#define LIC_TIME_MON	03
#define LIC_TIME_DAY	05
#define LIC_TIME_HOUR	12

class CLic
{
public:

	CLic();
	~CLic();

	volatile LONG m_lLicenceFlag;
private:

	time_t current_time;

	time_t finish_time;	// limit time working application
};

