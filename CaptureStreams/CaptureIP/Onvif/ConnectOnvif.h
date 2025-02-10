#pragma once

#include <winsock2.h>


#include <windows.h>

#include <iostream>
#include <fstream>
#include <string>

#pragma comment(lib, "ws2_32.lib")


#pragma warning( disable : 4996)

class CConnectOnvif
{
public:
	CConnectOnvif();
	~CConnectOnvif();

	int Init(char* ip_str, char* port_str, char* ip_tail);

	// get flag connect to IP CAMERA by ONVIF protocol
	long	GetFlagConnect(void) const;

	int Terminate();

	//////////////////////////////////////////////////////////////////////////
	// thread for getting WEB data (ONVIF protocol)
	static DWORD Func_GetData_ONVIF_Thread(LPVOID lpParam);

private:
	void GenStr_ONVIF__OPTIONS(char* options, char* http, int CSeq);
	void GenStr_ONVIF__DESCRIBE(char *describe, char *http, int CSeq);
	void GenStr_ONVIF__PLAY(char* play, char* http, int CSeq);

private:
	// timeout thread connection til IP camera (by ONVIF protocol) [MS]
	DWORD m_timeout_thread_connect_ms;

	SOCKET m_Socket_TCP;

	SOCKADDR_IN SockAddr;

	// Thread parameters
	HANDLE		m_hWebThread;
	DWORD			m_dwWebThreadId;

	//flag define net ONVIF protocol connection possibility and get data from SERVER (IP CAMERA)
	volatile LONG		m_bFlag_Thread_Finished;

	char m_IP_address_str[_MAX_PATH];
	char m_IP_port[_MAX_PATH];
	char m_IP_tail[_MAX_PATH];


};

