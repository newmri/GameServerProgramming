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
	const bool& Init(const HWND&);
	const bool& ProcessPacket(const HWND&, const UINT&, const WPARAM&, const LPARAM&);
	void ProcessPacket(char*);
	const bool& Connect(const HWND&);
	void ReadPacket();
	void SendMovePacket();
	void SendLoginPacket(char a_ID[], char a_PWD[]);
	const POINT& GetPos();
	void Close(const bool&);

public:
	void SetDlg(const HWND& a_hDlg) { m_hDlg = a_hDlg; }
public:
	stClientInfo m_stClientInfo[MAX_PLAYER];
	stNPCInfo m_stNPCInfo[MAX_NPC_NUM];

private:
	HWND m_hWnd;
	HWND m_hDlg;
private:
	bool m_First;

protected:
	int m_nPacketType;
	int m_nClientCnt;
	POINT m_pos;
	WORD m_wId;
	char m_ID[ID_LEN];
	bool m_IsMoved;
	bool m_IsLogined;
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

public:
	CImage m_PlayerImg;
	CImage m_NPCImg;
};