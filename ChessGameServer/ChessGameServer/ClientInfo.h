#pragma once

#include "OverLappedEx.h"
#define NAME_LEN 10

struct stClientInfo {
	SOCKADDR_IN m_saClientAddr; // Store client's information of address
	SOCKET m_SocketClient; // Client Socket

	stOverlappedEx m_stRecvOverlappedEx; // The variable to recv Overlapped I/O Working

	unsigned char m_szPacketBuf[MAX_PACKET_SIZE];
	WORD wPrevPacketData, wCurrPacketSize;

	char m_ID[NAME_LEN];
	bool m_bIsConnected;
	bool m_bIsLogined;

	POINT m_pos;
	unordered_set<WORD> m_view_list;
	unordered_set<WORD> m_NPC_view_list;
	mutex m_lock;
	mutex m_NPC_Lock;

	// Initialize member's variables
	stClientInfo()
	{
		ZeroMemory(&m_stRecvOverlappedEx, sizeof(stOverlappedEx));
		ZeroMemory(&m_saClientAddr, sizeof(SOCKADDR_IN));
		ZeroMemory(m_szPacketBuf, sizeof(m_szPacketBuf));

		m_SocketClient = INVALID_SOCKET;
		m_bIsConnected = false;
		m_bIsLogined = false;
		m_pos.x = CHESS_FIRST_X, m_pos.y = CHESS_FIRST_Y;

		wPrevPacketData = 0;
		wCurrPacketSize = 0;

	}
};

