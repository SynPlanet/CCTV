#include "StdAfx.h"
#include "NetSendCmr360.h"



/*
// Protocol Pelco-D for PTZ Camera

Description Protocol Pelco-D
|  byte 1  |   byte 2   |  byte 3  |  byte 4  | byte 5 | byte 6 | byte 7 |
 ---------- ------------ ---------- ---------- -------- -------- --------
|Sync 0xFF | Cam Adress | Command1 | Command2 | Data 1 | Data 2 |Checksum|


Description Commands
___________________________________________________________________________________________________________________________
|						| Bit 7			|	Bit 6			|	Bit 5			|	Bit 4							 |		Bit 3				|	Bit 2			|	Bit 1			| Bit 0			 |
 --------------------------------------------------------------------------------------------------------------------------
| Command 1	| Sense			|	Reserved	| Reserved	|	Auto / Manual Scan | Camera On/Off	| Iris Close|	Iris Open |	Focus Near |
 --------------------------------------------------------------------------------------------------------------------------
| Command 2	| Focus Far	|	Zoom Wide	|	Zoom Tele	| Tilt	Down				 |	Tilt Up				|	Pan Left	|	Pan Right	| Fixed to 0 |

*/
const unsigned char g_StopMoveCamera_PelcoD[7] = {0xFF, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01};


DWORD /*WINAPI*/ CNetSendCmr360::___SendNetDataThread(LPVOID lpParam)
{
	CNetSendCmr360 *p_NetSendCmr360 = (CNetSendCmr360 *)lpParam;

	// just in case
	if (!p_NetSendCmr360){
		return 1;
	}
    // Declare and initialize variables
    WSADATA wsaData;
    WSABUF DataBuf;

    WSAOVERLAPPED Overlapped;
    SOCKET SendToSocket = INVALID_SOCKET;

    struct sockaddr_in RecvAddr;
    struct sockaddr_in LocalAddr;
    int RecvAddrSize = sizeof (RecvAddr);
    int LocalAddrSize = sizeof (LocalAddr);

    u_short Port = 27777;
    struct hostent *localHost;
    char *ip;
    
    char *targetip;
    char *targetport;

    char SendBuf[1024] = "Data buffer to send";
    int BufLen = 1024;
    DWORD BytesSent = 0;
    DWORD Flags = 0;

    int rc, err;
    int retval = 0;

		char  szServerName[_MAX_PATH], szMessage[_MAX_PATH], ip_port[_MAX_PATH];
		long size_data_sendto = 0;
		//strcpy(szServerName, "127.0.0.1");
		strcpy(szServerName, "192.168.0.149");//strcpy(szServerName, "admin:1234@192.168.0.149:4520");
		strcpy(ip_port, "80");

    targetip = szServerName;
    targetport = ip_port;

    //---------------------------------------------
    // Initialize Winsock
    // Load Winsock
    rc = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (rc != 0) {
        printf("Unable to load Winsock: %d\n", rc);
        return 1;
    }

    // Make sure the Overlapped struct is zeroed out
    SecureZeroMemory((PVOID) &Overlapped, sizeof(WSAOVERLAPPED));

    // Create an event handle and setup the overlapped structure.
    Overlapped.hEvent = WSACreateEvent();
    if (Overlapped.hEvent == WSA_INVALID_EVENT) {
        printf("WSACreateEvent failed with error: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }
    //---------------------------------------------
    // Create a socket for sending data
    SendToSocket =
        WSASocket(AF_INET, SOCK_STREAM/*SOCK_DGRAM*/, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
    if (SendToSocket == INVALID_SOCKET) {
        printf("socket failed with error: %d\n", WSAGetLastError());
        WSACloseEvent(Overlapped.hEvent);
        WSACleanup();
        return 1;
    }
    //---------------------------------------------
    // Set up the RecvAddr structure with the IP address of
    // the receiver (in this example case "123.123.123.1")
    // and the specified port number.
    RecvAddr.sin_family = AF_INET;

    RecvAddr.sin_addr.s_addr = inet_addr(targetip);
    if (RecvAddr.sin_addr.s_addr == INADDR_NONE)  {
        printf("The target ip address entered must be a legal IPv4 address\n");
        WSACloseEvent(Overlapped.hEvent);
        WSACleanup();
        return 1;
    }
    RecvAddr.sin_port = htons( (u_short) atoi(targetport));
    if(RecvAddr.sin_port == 0) {
        printf("The targetport must be a legal UDP port number\n");
        WSACloseEvent(Overlapped.hEvent);
        WSACleanup();
        return 1;
    }

    //---------------------------------------------
    // Set up the LocalAddr structure with the local IP address
    // and the specified port number.
    localHost = gethostbyname("");
    ip = inet_ntoa(*(struct in_addr *) *localHost->h_addr_list);

    LocalAddr.sin_family = AF_INET;
    LocalAddr.sin_addr.s_addr = inet_addr(ip);
    LocalAddr.sin_port = htons(Port);

    //---------------------------------------------
    // Bind the sending socket to the LocalAddr structure
    // that has the internet address family, local IP address
    // and specified port number.  
    rc = bind(SendToSocket, (struct sockaddr *) &LocalAddr, LocalAddrSize);
    if (rc == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        WSACloseEvent(Overlapped.hEvent);
        closesocket(SendToSocket);
        WSACleanup();
        return 1;
    }
    //---------------------------------------------

//    printf("Sending a datagram...\n");
		unsigned char a[7] = {0xFF, 0x01, 0x88, 0x00, 0x00, 0x00, 0x89};		//Camera on
		unsigned char b[7] = {0xFF, 0x01, 0x00, 0x04, 0x00, 0x20, 0x25};		//Left 1/2 speed


    DataBuf.len = sizeof(b);// BufLen
    DataBuf.buf = (char*)b; //SendBuf;

    rc = WSASendTo(SendToSocket, &DataBuf, 1, &BytesSent, Flags, (SOCKADDR *) & RecvAddr,
                   RecvAddrSize, &Overlapped, NULL);

    if ((rc == SOCKET_ERROR) && (WSA_IO_PENDING != (err = WSAGetLastError()))) {
        printf("WSASendTo failed with error: %d\n", err);
        WSACloseEvent(Overlapped.hEvent);
        closesocket(SendToSocket);
        WSACleanup();
        return 1;
    }

    rc = WSAWaitForMultipleEvents(1, &Overlapped.hEvent, TRUE, INFINITE, TRUE);
    if (rc == WSA_WAIT_FAILED) {
        printf("WSAWaitForMultipleEvents failed with error: %d\n",
                WSAGetLastError());
        retval = 1;
    }

    rc = WSAGetOverlappedResult(SendToSocket, &Overlapped, &BytesSent,
                                FALSE, &Flags);
    if (rc == FALSE) {
        printf("WSASendTo failed with error: %d\n", WSAGetLastError());
        retval = 1;
    }
    else
        printf("Number of sent bytes = %d\n", BytesSent);
        
    //---------------------------------------------
    // When the application is finished sending, close the socket.
    printf("Finished sending. Closing socket.\n");
    WSACloseEvent(Overlapped.hEvent);
    closesocket(SendToSocket);
    printf("Exiting.\n");

    //---------------------------------------------
    // Clean up and quit.
    WSACleanup();

  return 0;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
DWORD /*WINAPI*/ CNetSendCmr360::SendNetDataThread(LPVOID lpParam)
{
	CNetSendCmr360 *p_NetSendCmr360 = (CNetSendCmr360 *)lpParam;

	// just in case
	if (!p_NetSendCmr360){
		return 1;
	}

	WSADATA       wsd;
	if (WSAStartup(MAKEWORD(2,2), &wsd) != 0)
	{
		//MessageBox(0, "Can't load WinSock", "Error", 0);
		return 99;
	}

	SOCKET        sSocket;
	struct sockaddr_in servaddr;
	char  szServerName[1024], szMessage[1024];
	struct hostent    *host = NULL;
	int err = 0;
	long size_data_sendto = 0;

	//strcpy(szMessage, "This is message from client");
	if (!strlen(p_NetSendCmr360->m_addressIpCmr360)){
		return 99;
	}
	//strcpy(szServerName, "127.0.0.1");
	strcpy(szServerName, "192.168.0.149");//strcpy(szServerName, "admin:1234@192.168.0.149:4520");

	size_data_sendto = sizeof(p_NetSendCmr360->m_Protocol_D);

	//sSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	sSocket = socket(AF_INET, SOCK_STREAM, /*0*/IPPROTO_TCP);

	if (sSocket == INVALID_SOCKET)
	{
		//MessageBox(0, "Can't create socket", "Error", 0);
		WSACleanup();
		return 99;
	}

	/////////////////////
	int n = 1;
// 	if (setsockopt(sSocket, SOL_SOCKET, SO_KEEPALIVE, (const char*) &n,	sizeof(n))) {
// 			perror("setsockopt(SO_KEEPALIVE)");
// 			exit(1);
// 	}

	/////////////////////

	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(80/*5050*/);
	servaddr.sin_addr.s_addr = inet_addr(szServerName/*p_NetSendCmr360->m_addressIpCmr360*/);

	// test
	if( (err = connect(sSocket,(struct sockaddr*) &servaddr, sizeof(servaddr)) ) == SOCKET_ERROR )
	{
		err = -1; //error
	}
	// test

	if (servaddr.sin_addr.s_addr == INADDR_NONE)
  {
      host = gethostbyname(szServerName/*p_NetSendCmr360->m_addressIpCmr360*/);
      if (host == NULL)
      {
					//MessageBox(0, "Unable to resolve server", "Error", 0);
          return 99;
      }
      CopyMemory(	&servaddr.sin_addr,
									host->h_addr_list[0],
									host->h_length
								);
  }

	unsigned char Checksum_loc = 0;

	while (p_NetSendCmr360->m_bFlagNetSend)
	{
		Checksum_loc = 0;

		for (int nmb_byte = 1; nmb_byte < 6; nmb_byte++){
			//calculate check sum
			Checksum_loc += p_NetSendCmr360->m_Protocol_D[nmb_byte];
		}
		p_NetSendCmr360->m_Protocol_D[6] = Checksum_loc % 256;
		
		unsigned char *p_ = p_NetSendCmr360->m_Protocol_D;

		unsigned char a[7] = {0xFF, 0x01, 0x88, 0x00, 0x00, 0x00, 0x89};		//Camera on
		unsigned char b[7] = {0xFF, 0x01, 0x00, 0x04, 0x00, 0x20, 0x25};		//Left 1/2 speed

					err = sendto(	sSocket,
										(const char *)&a[0],
										7,
										0,
										(struct sockaddr *)&servaddr,
										sizeof(sockaddr)
									);

		DWORD dwBytesWritten = 0;

		char buff[10] = "";
		int nsize = 0;
		if( (nsize = recv(sSocket, &buff[0], sizeof(buff), 0)) != SOCKET_ERROR)
		{
			err = 0;
		}
		err = WSAGetLastError();


			err = sendto(	sSocket,
										(const char *)&b[0],
										7,
										0,
										(struct sockaddr *)&servaddr,
										sizeof(sockaddr)
									);

//		for (int i = 0; i < 7; i++){
// 			err = sendto(	sSocket,
// 										(const char *)(p_NetSendCmr360->m_Protocol_D[i]),
// 										1/*size_data_sendto*/,
// 										0,
// 										(struct sockaddr *)&servaddr,
// 										sizeof(servaddr)
// 									);
// 						err = send(	sSocket,
// 												(const char *)(p_NetSendCmr360->m_Protocol_D[i]),
// 												1/*size_data_sendto*/,
// 												0
// 											);
// 
// 			if(err == SOCKET_ERROR){
// 				int e = 9;
// 			}
// 
// 
// 			err = WSAGetLastError();
// 		}

/*
		err = sendto(	sSocket,
						szMessage,
						30,
						0,
						(struct sockaddr *)&servaddr,
						sizeof(servaddr)
					   );
*/
	}
	WSACleanup();

  return 0;
}

CNetSendCmr360::CNetSendCmr360(void)
{
	m_bFlagNetSend = false;
	m_hNetThread = NULL;
	m_dwNetThreadId = NULL;

	// set base value for stop camera 
	memcpy_s(m_Protocol_D, sizeof(m_Protocol_D), g_StopMoveCamera_PelcoD, sizeof(g_StopMoveCamera_PelcoD));
}


CNetSendCmr360::~CNetSendCmr360(void)
{
	Release();
}

int CNetSendCmr360::Init(const char* inet_address_in)
{
	sprintf_s(m_addressIpCmr360, _MAX_PATH, "%s", inet_address_in);

	// check length address IP camera
	if (!strlen(m_addressIpCmr360)){
		return -1;
	}

	m_hNetThread = CreateThread(	NULL,
																0,
																(LPTHREAD_START_ROUTINE)SendNetDataThread, 
																(void*)this,
																0,
																&m_dwNetThreadId
															);
	if (m_hNetThread){
		// set accuracy processor for the created thread
		//DWORD err_ = SetThreadIdealProcessor(m_hGrabThread, m_nmb_Processor);

		bool err = SetThreadPriority(m_hNetThread, THREAD_PRIORITY_LOWEST);

		m_bFlagNetSend = true;

		return 0;
	}else{
		return -1;
	}

	return -2;
}

bool CNetSendCmr360::IsEnable(void) const
{
	return m_bFlagNetSend;
}

int CNetSendCmr360::Release(void)
{
	// terminate thread
	if (m_hNetThread){
		ResumeThread(m_hNetThread);

		m_bFlagNetSend = false;

		// take time for thread to resume work
		Sleep(250);

		WaitForSingleObject(m_hNetThread, INFINITE);

		TerminateThread(m_hNetThread, NULL/*STILL_ACTIVE*/);

		CloseHandle(m_hNetThread);
		m_hNetThread = NULL;

		return 0;
	}
	return -1;
}

//////////////////////////////////////////////////////////////////////////
// data was gotten from joystick for net sending
void CNetSendCmr360::SetJoyControl( const T_DataPTZ			*p_data_PTZ, 
																		const unsigned char type_device							//T_JoyDevice::enum
																	)
{
	// decoding protocol from joystick for translate it to Pelco-D protocol
	Translate2PelcoD(p_data_PTZ, type_device);

	if (type_device == T_JoyDevice::VIRTUAL){
		;
	}else{
		;
	}
}

//////////////////////////////////////////////////////////////////////////
// translate protocol from structure T_DataPTZ to protocol Pelco-D
void CNetSendCmr360::Translate2PelcoD(	const T_DataPTZ *p_data_PTZ,
																				const unsigned char type_device							//T_JoyDevice::enum
																			)
{
	//just in case
	if(!p_data_PTZ){
		return;
	}
	const unsigned char focusNull = 0x00;  // «ума нет
	ZeroMemory((void*)m_Protocol_D, sizeof(m_Protocol_D) );

	m_Protocol_D[0] = 0xFF;	// sync bit
	m_Protocol_D[1] = 0x01;	// camera number
	m_Protocol_D[2] = focusNull;	// Command_1
	m_Protocol_D[4] = focusNull;	// Data_1 for the Command_1

	// command 2
	if (p_data_PTZ->state_controle & T_ControlPTZ::DOWN){
		m_Protocol_D[3] |= T_Command2_PelcoD::TILT_DOWN;
	}else{
		if (p_data_PTZ->state_controle & T_ControlPTZ::UP){
			m_Protocol_D[3] |= T_Command2_PelcoD::TILT_UP;
		}
	}
	if (p_data_PTZ->state_controle & T_ControlPTZ::LEFT){
		m_Protocol_D[3] |= T_Command2_PelcoD::PAN_LEFT;
	}else{
		if (p_data_PTZ->state_controle & T_ControlPTZ::RIGHT){
			m_Protocol_D[3] |= T_Command2_PelcoD::PAN_RIGHT;
		}
	}
	if (p_data_PTZ->state_controle & T_ControlPTZ::ZOOM_IN){
		m_Protocol_D[3] |= T_Command2_PelcoD::ZOOM_TELE;
	}else{
		if (p_data_PTZ->state_controle & T_ControlPTZ::ZOOM_OUT){
			m_Protocol_D[3] |= T_Command2_PelcoD::ZOOM_WIDE;
		}
	}

	// Data_2 for the Command_2
	//m_Protocol_D[5] = 0x3F;

	if (p_data_PTZ->state_controle & T_ControlPTZ::SPEED_UP){
		//m_Protocol_D[5] = 0xFF;
		m_Protocol_D[5] = 0x3F;
	}else{
		// get bigger speed
		float speed_joy = 0x00;
		if (type_device == T_JoyDevice::VIRTUAL){
			speed_joy = 0x3F;	// 0x3F === 63
		}else{
			speed_joy = (p_data_PTZ->speed_LeftRight < p_data_PTZ->speed_moving) ? p_data_PTZ->speed_moving : p_data_PTZ->speed_LeftRight;
			speed_joy = (float)0x3F * speed_joy;
		}
		m_Protocol_D[5] = (unsigned char)( speed_joy );
	}

	const unsigned char tiltDown = 0x10;
	const unsigned char tiltUp = 0x08;
	const unsigned char panRight = 0x02;
	const unsigned char panLeft = 0x04;
	const unsigned char focusNear = 0x20;  // «ум +  0x01
	const unsigned char focusFar = 0x40;   // «ум -  0x80

	m_Protocol_D[3] = tiltUp;
	m_Protocol_D[5] = 0x3F;


}