#pragma once

#include "OverLappedEx.h"

struct stClientInfo {
	SOCKADDR_IN m_saClientAddr; // Store client's information of address
	SOCKET m_SocketClient; // Client Socket

	stOverlappedEx m_stRecvOverlappedEx; // The variable to recv Overlapped I/O Working

	unsigned char m_szPacketBuf[MAX_PACKET_SIZE];
	WORD wPrevPacketData, wCurrPacketSize;

	bool m_bIsConnected;
	POINT m_pos;
	unordered_set<WORD> m_view_list;
	mutex m_lock;
	// Initialize member's variables
	stClientInfo()
	{
		ZeroMemory(&m_stRecvOverlappedEx, sizeof(stOverlappedEx));
		ZeroMemory(&m_saClientAddr, sizeof(SOCKADDR_IN));
		ZeroMemory(m_szPacketBuf, sizeof(m_szPacketBuf));

		m_SocketClient = INVALID_SOCKET;
		m_bIsConnected = false;
		m_pos.x = CHESS_FIRST_X, m_pos.y = CHESS_FIRST_Y;

		wPrevPacketData = 0;
		wCurrPacketSize = 0;

	}
};

