#include "StdAfx.h"
#include "Proccesor.h"


CProccesor::CProccesor(void)
{
	m_cnt_new_procsr = 0;
	GetInfo();
}

CProccesor::~CProccesor(void)
{
}

void CProccesor::GetInfo(void)
{
	GetSystemInfo(&m_sys_info);	//	GetNativeSystemInfo (&m_sys_info);
}

// get number processor for the thread
unsigned int CProccesor::GetNmb_Processor4Thread(void)
{
	if (m_sys_info.dwNumberOfProcessors == 1){
		return 0;
	}

	int nmb_loc = m_cnt_new_procsr % m_sys_info.dwNumberOfProcessors;

	if (nmb_loc == 0){
		nmb_loc ++;
		m_cnt_new_procsr++;
	}

	m_cnt_new_procsr++;

	return nmb_loc;
}