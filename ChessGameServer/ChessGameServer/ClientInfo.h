#pragma once

#include "OverLappedEx.h"

struct stClientInfo {
	SOCKADDR_IN m_saClientAddr; // Store client's information of address
	SOCKET m_SocketClient; // Client Socket
	enumLocation m_eLocation; // Client Location
	stOverlappedEx m_stRecvOverlappedEx; // The variable to recv Overlapped I/O Working
	stOverlappedEx m_stSendOverlappedEx; // The variable to send Overlapped I/O Working
	POINT m_pos;
	// Initialize member's variables
	stClientInfo()
	{
		ZeroMemory(&m_stRecvOverlappedEx, sizeof(stOverlappedEx));
		ZeroMemory(&m_stSendOverlappedEx, sizeof(stOverlappedEx));
		ZeroMemory(&m_saClientAddr, sizeof(SOCKADDR_IN));
		m_SocketClient = INVALID_SOCKET;
		m_eLocation = eLOBBY;
		m_pos.x = CHESS_FIRST_X, m_pos.y = CHESS_FIRST_Y;
	}
};