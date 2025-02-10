#pragma once

#include <winsock2.h>
#include "CommonIP.h"

using namespace IP_Camera;
using namespace JoyDevice;

class CNetSendCmr360
{
public:
	CNetSendCmr360(void);
	~CNetSendCmr360(void);

	int Init(const char* inet_address_in);

	int Release(void);

	bool IsEnable(void) const;

	// threading function
	static DWORD ___SendNetDataThread(LPVOID lpParam);
	// threading function
	static DWORD SendNetDataThread(LPVOID lpParam);
	// data was gotten from joystick for net sending
	void SetJoyControl( const T_DataPTZ			*p_data_PTZ, 
											const unsigned char type_device							//T_JoyDevice::enum
										);
private:

 /*volatile*/ char	m_addressIpCmr360[_MAX_PATH];

 // translate protocol from structure T_DataPTZ to protocol Pelco-D
 void Translate2PelcoD(	const T_DataPTZ *p_data_PTZ,
												const unsigned char type_device							//T_JoyDevice::enum
											);

 //flag define net connecting possibility and send data to receiver
 bool	m_bFlagNetSend;

 HANDLE	m_hNetThread;
 DWORD   m_dwNetThreadId;

 unsigned char m_Protocol_D[7];
};

