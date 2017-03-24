#pragma once

#include <WinSock2.h>

#include "Define.h"

#pragma comment(lib,"ws2_32")
#pragma comment(lib, "user32.lib")

#define SERVER_PORT 9000
#define WM_SOCKET WM_USER+1

class CTransmission
{
public:
	CTransmission();
	~CTransmission();
	void SetServerIP(const HWND&);
	bool Init(const HWND&);
	void ProcessSocketMessage(const HWND&, const UINT&, const WPARAM&, const LPARAM&);
	bool Connect();
	void Close(bool);

protected:
	TCHAR m_tchServerIp[MAX_IP_LEN];
	SOCKET m_sock;
	SOCKADDR_IN m_saServerAddr;
};