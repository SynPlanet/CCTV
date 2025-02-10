#include "StdAfx.h"
#include <winsock2.h>
#include "NetControlReceive.h"
#include "CommonIP.h"
#include "Extern.h"

#include <stdio.h>

using namespace IP_Camera;

//extern ListCamerasIP	gList_Cameras_IP;

extern Set_PlayerState p_Set_PlayerState;

// set name folder for the movie playing
extern SetMovieFolder	p_SetMovieFolder;

//////////////////////////////////////////////////////////////////////////
DWORD CNetControlReceive::SetNetDataToClient(void)
{
	char data_to_client[4];
	int iResult;

	itoa((int) m_lClientData, data_to_client, 10);

	iResult = send( sAcceptSocket, data_to_client, (int)strlen(data_to_client), 0 );

	if (iResult == SOCKET_ERROR) {
		printf("send failed: %d\n", WSAGetLastError());

		InterlockedExchange(&m_bFlagNetEnable, false);
		return 101;
	}

	// shutdown the connection since we're done
	iResult = shutdown(sAcceptSocket, SD_SEND);
	if(iResult == SOCKET_ERROR){
		printf("send failed: %d\n", WSAGetLastError());
		InterlockedExchange(&m_bFlagNetEnable, false);
		return 102;
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////d
DWORD /*WINAPI*/ CNetControlReceive::GetNetDataThread(LPVOID lpParam)
{

	CNetControlReceive *p_netCtrlRecv = (CNetControlReceive *)lpParam;

	// just in case
	if (!p_netCtrlRecv){

		InterlockedExchange(&p_netCtrlRecv->m_bFlagNetEnable, false);

		return 1;
	}

// 	SOCKET        sListenSocket;
// 	SOCKET        sAcceptSocket;
	struct sockaddr_in	localaddr, 
											clientaddr;
	int      iSize_Client;
	hostent* localHost;
	char* localIP;

	int err = 0;

	// Initialize Winsock
	WSADATA wsaData;
	int rc;

	rc = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (rc != 0) {
		printf("Unable to load Winsock: %d\n", rc);
		return 1;
	}

	// Create a listening socket
	p_netCtrlRecv->sListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  if (p_netCtrlRecv->sListenSocket == INVALID_SOCKET)
	{
		int err = WSAGetLastError();
		printf("Can't create socket 'class NetControlReceive' \n");
		WSACleanup();
		return 91;
	}

	// Get the local host information
	localHost = gethostbyname("");
	localIP = inet_ntoa (*(struct in_addr *)*localHost->h_addr_list);

	// Set up the localaddr structure
	localaddr.sin_addr.s_addr = htonl(INADDR_ANY);//	localaddr.sin_addr.s_addr = inet_addr(localIP);
	localaddr.sin_family = AF_INET;
	localaddr.sin_port = htons(p_netCtrlRecv->m_NetPort_listen);

	// Bind the listening socket using the information in the sockaddr structure
	if (::bind(p_netCtrlRecv->sListenSocket, (struct sockaddr *)&localaddr, sizeof(localaddr)) == SOCKET_ERROR)
	{
		closesocket(p_netCtrlRecv->sListenSocket);
		WSACleanup();
		return 92;
	}

	// Listen for incoming connection requests on the created socket
	if(listen( p_netCtrlRecv->sListenSocket, SOMAXCONN) == SOCKET_ERROR){
		closesocket(p_netCtrlRecv->sListenSocket);
		WSACleanup();
		return 93;
	}

	//set permits an incoming connection attempt on a socket
	iSize_Client = sizeof(clientaddr);
	p_netCtrlRecv->sAcceptSocket = accept(p_netCtrlRecv->sListenSocket, (struct sockaddr*)&clientaddr, &iSize_Client);

	if (p_netCtrlRecv->sAcceptSocket == INVALID_SOCKET){
		InterlockedExchange(&p_netCtrlRecv->m_bFlagNetEnable, false);

		closesocket(p_netCtrlRecv->sListenSocket);
		WSACleanup();

		return 94;
	}else{
		// set connecting flag -> !!!OK!!!
		InterlockedExchange(&p_netCtrlRecv->m_bFlagNetEnable, true);
	}

	char cVal_loc[2]; // для прочтения только 1 символа с последующей его конвертацией создаём массив из 2 CHAR

//	char cDummy_loc = 32;
	unsigned int uiShiftTimePlay = 0;
	int iCmd;
	int ret = 0;
	int iResult = 0;

	const unsigned int nmb_char_max = MAX_SIGN_NAME_FOLDER;	//максимальный размер буфера для чтения по сети (ограничился 32 байтами/символами)
	char recv_buf[nmb_char_max] = ""; // массив байт для получения данных (ПО ПРОТОКОЛУ!!!)
	
	while (InterlockedCompareExchange(&p_netCtrlRecv->m_bFlagNetEnable, 1,1))
	{
		// обнуляем буфер!!! -> второй символ делаем закрывающем строку (чтобы не было склеивания строки с символом с предыдущего этапа!!! -> ошибка при конвертации ATOI() )
		cVal_loc[0] = '255';
		cVal_loc[1] = '\0';

		// устанавливаем значение по умолчанию
		uiShiftTimePlay = 0;
		int sz_buf_recv = 0;
		ZeroMemory((void*)recv_buf, nmb_char_max * sizeof(char));

		// the first sign indicated the command
		if (recv(p_netCtrlRecv->sAcceptSocket, (char *)cVal_loc, sizeof(char), 0 ) > 0){
			
			iCmd = atoi((const char *)cVal_loc);

			// save data was get from client
			InterlockedExchange(&p_netCtrlRecv->m_lClientData, iCmd);

// 			char log[32];
// 			sprintf(log, "%d", iCmd);
// 			MessageBoxA(NULL, log, "recv", MB_OK);
							// change recording state 
			// change recording state 
			UpdataStateStream(	-1,		// number of current camera
													iCmd	// the flag of writing the video streaming from Camera
												);

			switch (iCmd)
			{
				// command for exit application
				case ((LONG)T_DeviceState::EXIT):
					{
						if (p_Set_PlayerState){
							printf("!!! EXIT !!!\n");
							// run callback function up to *.EXE
							p_Set_PlayerState(UINT(-1), T_DeviceState::EXIT);
						}
					}
					break;

				case ((LONG)T_DeviceState::PAUSE):
					{
						// old version
						//if (recv(p_netCtrlRecv->sAcceptSocket, (char *)(&uiShiftTimePlay), sizeof(uiShiftTimePlay), 0) > 0) {
						//uiShiftTimePlay = atoi((const char *)&uiShiftTimePlay);
						
						// получение времени(смещения от начала проигрывания) в режиме ПАУЗА
						if ((sz_buf_recv = recv(p_netCtrlRecv->sAcceptSocket, (char *)(recv_buf), nmb_char_max * sizeof(char), 0)) > 0) {

							recv_buf[sz_buf_recv] = '\0';
							uiShiftTimePlay = atoi((const char *)recv_buf); // 
							printf("!!! Time PAUSE: %d\n", uiShiftTimePlay);

							SetTimePlay(uiShiftTimePlay*1000);
						}
					}
					break;

				case ((LONG)T_DeviceState::PLAY):
					{
						// получение времени(смещения от начала проигрывания) в режиме ПРОИГРЫВАНИЯ
						if ((sz_buf_recv = recv(p_netCtrlRecv->sAcceptSocket, (char *)(recv_buf), nmb_char_max * sizeof(char), 0)) > 0) {

							recv_buf[sz_buf_recv] = '\0';
							uiShiftTimePlay = atoi((const char *)recv_buf);
							printf("!!! Time PLAY: %d\n", uiShiftTimePlay);

							SetTimePlay(uiShiftTimePlay*1000);
						}
					}
					break;

				case ((LONG)T_DeviceState::MOUNT):
					{
						// получение имени папки для "монтирования"
						if ((sz_buf_recv = recv(p_netCtrlRecv->sAcceptSocket, (char *)(recv_buf), nmb_char_max * sizeof(char), 0 )) > 0){

							recv_buf[sz_buf_recv] = '\0';
							printf("!!! Get Name Folder to the mounting: %s\n", recv_buf);

							SetFolderNet(recv_buf);
						}
					}
					break;
			}

			//////////////////////////////////////////////////////////////////////////
			// send report to client if it's OK
			p_netCtrlRecv->SetNetDataToClient();
			//////////////////////////////////////////////////////////////////////////
		}else{

			ret = WSAGetLastError();
			// try to reconnected net joining
			if( (ret == WSAECONNABORTED) || (ret == WSAENOTCONN) || (ret == SOCKET_ERROR) || (ret == 0) ){

				// shutdown the connection since we're done
				iResult = shutdown(p_netCtrlRecv->sAcceptSocket, SD_RECEIVE);
				if(iResult == SOCKET_ERROR){
					closesocket(p_netCtrlRecv->sAcceptSocket);

					InterlockedExchange(&p_netCtrlRecv->m_bFlagNetEnable, false);
					continue;
				}
				p_netCtrlRecv->sAcceptSocket = accept(p_netCtrlRecv->sListenSocket, (struct sockaddr*)&clientaddr, &iSize_Client);
				if (p_netCtrlRecv->sAcceptSocket == INVALID_SOCKET){
					InterlockedExchange(&p_netCtrlRecv->m_bFlagNetEnable, false);
					continue;
				}
			}
		}
	}

	// shutdown the connection since we're done
	err = shutdown(p_netCtrlRecv->sAcceptSocket, SD_BOTH);
	if (err == SOCKET_ERROR) {
		;
	}
	// no longer need server socket -> cleanup
	closesocket(p_netCtrlRecv->sListenSocket);
	p_netCtrlRecv->sListenSocket = 0;	

	closesocket(p_netCtrlRecv->sAcceptSocket);
	p_netCtrlRecv->sAcceptSocket = 0;

	return 0;
}

//////////////////////////////////////////////////////////////////////////
CNetControlReceive::CNetControlReceive(void)
{
	m_NetPort_listen = 5050;

	InterlockedExchange(&m_bFlagNetEnable, false);
	InterlockedExchange(&m_lClientData, 0);
	
}

//////////////////////////////////////////////////////////////////////////
CNetControlReceive::~CNetControlReceive(void)
{
	Release();
}

//////////////////////////////////////////////////////////////////////////
int CNetControlReceive::InitGetCommandNet(unsigned int inet_port_listen_in)
{
	if (m_NetPort_listen == inet_port_listen_in){
		// this port are listened
		return 1;
	}else{
		Release();
	}
	m_NetPort_listen = inet_port_listen_in;

	m_hNetThread = CreateThread(	NULL,
																0,
																(LPTHREAD_START_ROUTINE)GetNetDataThread, 
																(void*)this,
																0,
																&m_dwNetThreadId
															);
	if (m_hNetThread){
		// set accuracy processor for the created thread
		//DWORD err_ = SetThreadIdealProcessor(m_hGrabThread, m_nmb_Processor);

		bool err = SetThreadPriority(m_hNetThread, THREAD_PRIORITY_LOWEST);

		return 0;
	}else{
		return -1;
	}

	return -2;
}

//////////////////////////////////////////////////////////////////////////
bool CNetControlReceive::IsEnable(void) const
{
	return m_bFlagNetEnable;
}

int CNetControlReceive::Release(void)
{
	// terminate thread
	if (m_hNetThread){
		InterlockedExchange(&m_bFlagNetEnable, false);

		ResumeThread(m_hNetThread);

		// take time for correct finish thread work
		if (WAIT_TIMEOUT == WaitForSingleObject(m_hNetThread, 1000)){
			;
		}
		TerminateThread(m_hNetThread, NULL);
		CloseHandle(m_hNetThread);
		m_hNetThread = NULL;

		if (sListenSocket){	// just in case
			closesocket(sListenSocket);
		}
		if (sAcceptSocket){	// just in case
			shutdown(sAcceptSocket, SD_BOTH);
			closesocket(sAcceptSocket);
		}

		// Clean up and quit.
		WSACleanup();

		return 0;
	}
	return -1;
}

//////////////////////////////////////////////////////////////////////////