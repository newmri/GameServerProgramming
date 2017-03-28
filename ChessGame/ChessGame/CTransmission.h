#pragma once

#include <WinSock2.h>

#include "stPlayerInfo.h"

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
	bool ProcessSocketMessage(const HWND&, const UINT&, const WPARAM&, const LPARAM&);
	bool Connect(const HWND&);
	bool Recv(char*, int, int);
	int Recvn(char*, int, int);
	void AssembleAndSendPacket(enumDataType&);
	bool Send(char*, int);
	POINT GetPos();
	void Close(bool);

protected:
	TCHAR m_tchServerIp[MAX_IP_LEN];
	SOCKET m_sock;
	SOCKADDR_IN m_saServerAddr;
	char m_szBuf[MAX_BUF_SIZE];
	int m_nDataLen;
	POINT m_pos;
	stPlayerInfo m_stPlayerInfo[MAX_PLAYER];
};