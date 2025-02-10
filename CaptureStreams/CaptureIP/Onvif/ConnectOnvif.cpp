#include "stdafx.h"
#include "ConnectOnvif.h"
using namespace std;



CConnectOnvif::CConnectOnvif()
{
	m_timeout_thread_connect_ms = 500;

	InterlockedExchange(&m_bFlag_Thread_Finished, 0);
}


CConnectOnvif::~CConnectOnvif()
{
}

int CConnectOnvif::Init(char* ip_str, char* port_str, char* ip_tail)
{
	sprintf_s(m_IP_address_str, _MAX_PATH, ip_str);
	sprintf_s(m_IP_port, _MAX_PATH, port_str);
	sprintf_s(m_IP_tail, _MAX_PATH, ip_tail);
	
	m_Socket_TCP = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	InterlockedExchange(&m_bFlag_Thread_Finished, 0);

	// create thread for web browsing
	m_hWebThread = CreateThread(	NULL,
																0,
																(LPTHREAD_START_ROUTINE)Func_GetData_ONVIF_Thread,
																(void*)this,
																0,
																&m_dwWebThreadId
															);

	return 0;
}

//////////////////////////////////////////////////////////////////////////////////
// thread for getting WEB data (ONVIF protocol)
DWORD CConnectOnvif::Func_GetData_ONVIF_Thread(LPVOID lpParam)
{
	CConnectOnvif *p_CConnectOnvif = (CConnectOnvif *)lpParam;

	// IP camera string request
	char str_request_CRM[_MAX_PATH] = "";
	char str_request_ONVIF[_MAX_PATH*4] = "";

	// forming string getting data request 
	sprintf_s(str_request_CRM, _MAX_PATH, "rtsp://%s:%s%s", p_CConnectOnvif->m_IP_address_str, p_CConnectOnvif->m_IP_port, p_CConnectOnvif->m_IP_tail);

	/////////////////////////
	struct hostent *host;

	host = gethostbyname((const char*)p_CConnectOnvif->m_IP_address_str);	//host = gethostbyname("www.google.com");

	SOCKADDR_IN SockAddr;
	SockAddr.sin_port = htons(atoi(p_CConnectOnvif->m_IP_port));
	SockAddr.sin_family = AF_INET;
	SockAddr.sin_addr.s_addr = *((unsigned long*)host->h_addr);
	//cout << "Try to connect...\n";

	if (connect(p_CConnectOnvif->m_Socket_TCP, (SOCKADDR*)(&SockAddr), sizeof(SockAddr)) != 0) {
		//cout << "Could not connect";
		return 100;
	}
	//cout << "Connected.\n";

	/////////////////////////

	const UINT max_str = 15000;
	char buf_client[max_str];
	int nDataLng_client = 0;
	int CSeq = 1, num = 0;


	// set string description "DESCRIBE"
	sprintf_s(str_request_ONVIF, _MAX_PATH*4, "");	// set string ONVIF request default
	sprintf_s(buf_client, max_str, "");	// set buffer recieved default


	p_CConnectOnvif->GenStr_ONVIF__DESCRIBE(str_request_ONVIF, str_request_CRM, CSeq++);
	send(p_CConnectOnvif->m_Socket_TCP, (const char*)str_request_ONVIF, (int)strlen(str_request_ONVIF), 0);

	if ( (nDataLng_client = recv(p_CConnectOnvif->m_Socket_TCP, buf_client, max_str, 0) ) > 0) {

		if (strstr(buf_client, "Not Found")) {
			return 101;
		}

		std::string str_parsing(buf_client, nDataLng_client);

		nDataLng_client = (int)str_parsing.size();
		//cout << str_parsing.c_str();
	} else {
		// there is not connect to IP cavera by ONVIF
		return 100;
	}
	///

	// set string description "OPTIONS"
	sprintf_s(str_request_ONVIF, _MAX_PATH*4, "");	// set string ONVIF request default
	sprintf_s(buf_client, max_str, "");	// set buffer recieved default

	p_CConnectOnvif->GenStr_ONVIF__OPTIONS(str_request_ONVIF, str_request_CRM, CSeq++);
	send(p_CConnectOnvif->m_Socket_TCP, (const char*)str_request_ONVIF, (int)strlen(str_request_ONVIF), 0);

	if ((nDataLng_client = recv(p_CConnectOnvif->m_Socket_TCP, buf_client, max_str, 0)) > 0) {

		if (strstr(buf_client, "Not Found")) {
			return 102;
		}

		std::string str_parsing(buf_client, nDataLng_client);

		nDataLng_client = (int)str_parsing.size();
		//cout << str_parsing.c_str();
	} else {
		// there is not connect to IP cavera by ONVIF
		return 100;
	}
	///

	/*
	// set string description "PLAY"
	sprintf_s(str_request_ONVIF, _MAX_PATH*4, "");	// set string ONVIF request default
	sprintf_s(buf_client, max_str, "");	// set buffer recieved default

	p_CConnectOnvif->GenStr_ONVIF__PLAY(str_request_ONVIF, str_request_CRM, CSeq++);
	send(p_CConnectOnvif->m_Socket_TCP, (const char*)str_request_ONVIF, (int)strlen(str_request_ONVIF), 0);

	if ((nDataLng_client = recv(p_CConnectOnvif->m_Socket_TCP, buf_client, max_str, 0)) > 0) {

		if (strstr(buf_client, "Not Found")) {
			return 103;
		}

		std::string str_parsing(buf_client, nDataLng_client);

		nDataLng_client = (int)str_parsing.size();
		cout << str_parsing.c_str();
	} else {
		// there is not connect to IP cavera by ONVIF
		return 100;
	}
	///
	*/
	InterlockedExchange(&p_CConnectOnvif->m_bFlag_Thread_Finished, 1);

	return 0;
}

//////////////////////////////////////////////////////////////////////////////////
// get flag connect to IP CAMERA by ONVIF protocol
long	CConnectOnvif::GetFlagConnect(void) const
{
	return m_bFlag_Thread_Finished;
}

//////////////////////////////////////////////////////////////////////////////////
void CConnectOnvif::GenStr_ONVIF__OPTIONS(char* options, char* http, int CSeq)
{
	char temp[128] = { 0 };
	sprintf(options, "OPTIONS %s RTSP/1.0\r\n", http);
	sprintf(temp, "CSeq: %d\r\n\r\n", CSeq);
	strcat(options, temp);
	/*	strcpy(temp, "User-Agent: NCTU-Client\r\n");
	strcat(options, temp);
	strcpy(temp, "RegionData:407\r\n\r\n");
	strcat(options, temp);
	*/
}

//////////////////////////////////////////////////////////////////////////////////
void CConnectOnvif::GenStr_ONVIF__DESCRIBE(char *describe, char *http, int CSeq)
{
	char temp[128] = { 0 };
	sprintf(describe, "DESCRIBE %s RTSP/1.0\r\n", http);
	sprintf(temp, "CSeq: %d\r\n\r\n", CSeq);
	strcat(describe, temp);
	/*strcpy(temp, "Accept: application/sdp\r\n");
	strcat(describe, temp);
	strcpy(temp, "User-Agent: NCTU-Client\r\n");
	strcat(describe, temp);
	strcpy(temp, "Bandwidth: 12345678\r\n");
	strcat(describe, temp);
	strcpy(temp, "Accept-Language: en-US\r\n");
	strcat(describe, temp);
	strcpy(temp, "ClientID: WinNT555\r\n");
	strcat(describe, temp);
	strcpy(temp, "GUID: 777\r\n");
	strcat(describe, temp);
	strcpy(temp, "Required: com.real\r\n\r\n");
	strcat(describe, temp);
	*/
}

//////////////////////////////////////////////////////////////////////////////////
void CConnectOnvif::GenStr_ONVIF__PLAY(char* play, char* http, int CSeq)
{
	char temp[128] = { 0 };
	char Session[40] = { 0 };

	sprintf(play, "PLAY %s RTSP/1.0\r\n", http);
	sprintf(temp, "CSeq: %d\r\n", CSeq);
	strcat(play, temp);
	/**/
	strcat(play, Session);
	strcpy(temp, "Range: npt=0-7.74000\r\n\r\n");
	strcat(play, temp);
	
}

//////////////////////////////////////////////////////////////////////////////////
int CConnectOnvif::Terminate(void)
{
	// take time for thread to resume work
	DWORD res = WaitForSingleObject(m_hWebThread, m_timeout_thread_connect_ms);

//	if (WAIT_TIMEOUT == res) {
		TerminateThread(m_hWebThread, NULL/*STILL_ACTIVE*/);

		CloseHandle(m_hWebThread);
		closesocket(m_Socket_TCP);

		m_Socket_TCP = NULL;

		m_hWebThread = NULL;
//	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////////
