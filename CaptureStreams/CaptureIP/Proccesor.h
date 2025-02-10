#pragma once
class CProccesor
{
public:
	CProccesor(void);
	~CProccesor(void);

	// get number processor for the thread
	unsigned int GetNmb_Processor4Thread(void);

private:

	void GetInfo(void);

	unsigned int m_cnt_new_procsr;	// counter or new processor number
	//		processor:	1 - 2 - 3 - 4
	//		threads			1 - 2 - 3 - 4
	//								5 - 6 - 7 - 8
	//								9 -10 - 11 -N

	SYSTEM_INFO m_sys_info;

};

