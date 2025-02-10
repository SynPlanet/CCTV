#pragma once
class CNetControlReceive
{
public:
	CNetControlReceive(void);
	virtual ~CNetControlReceive(void);

	int InitGetCommandNet(unsigned int inet_port_listen_in);

	int Release(void);

	bool IsEnable(void) const;

private:
	// threading function
	static DWORD GetNetDataThread(LPVOID lpParam);

	// set data to client net
	/*static*/ DWORD SetNetDataToClient(void);

	//flag define net connecting possibility and send data to receiver
	volatile LONG m_bFlagNetEnable;

	// saving data from client
	volatile LONG m_lClientData;

	HANDLE	m_hNetThread;
	DWORD   m_dwNetThreadId;

	unsigned int m_NetPort_listen;

	SOCKET        sListenSocket;
	SOCKET        sAcceptSocket;
};

