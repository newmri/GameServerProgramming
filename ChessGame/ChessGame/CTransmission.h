#pragma once

#include <WinSock2.h>

#include "stClientInfo.h"

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
	bool ProcessPacket(const HWND&, const UINT&, const WPARAM&, const LPARAM&);
	void ProcessPacket(char*);
	bool Connect(const HWND&);
	void ReadPacket();
	void SendPacket();
	bool Send(char*, int);
	const POINT& GetPos();
	void Close(bool);
public:
	stClientInfo m_stClientInfo[MAX_PLAYER];

private:
	bool m_First;

protected:
	int m_nPacketType;
	int m_nClientCnt;
	POINT m_pos;
	WORD m_wId;
	bool m_IsMoved;
private:
	TCHAR m_tchServerIp[MAX_IP_LEN];
	SOCKET m_sock;
	SOCKADDR_IN m_saServerAddr;

	WSABUF m_send_wsabuf;
	char m_send_buffer[MAX_BUF_SIZE];
	WSABUF m_recv_wsabuf;
	char m_recv_buffer[MAX_BUF_SIZE];
	char m_packet_buffer[MAX_BUF_SIZE];
	DWORD m_in_packet_size = 0;
	int	m_saved_packet_size = 0;

};