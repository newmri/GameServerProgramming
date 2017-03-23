#pragma once

#include "Define.h"

// WSAOVERLAPPED Expansion of structure 
struct stOverlappedEx {
	WSAOVERLAPPED m_wsaOverlapped; // Overlapped I/O Structure
	SOCKET m_SocketClient; // Client Socket
	WSABUF m_wsaBuf; // Overlapped I/O Working buffer
	char m_szBuf[MAX_BUF_SIZE]; // Data buffer
	enumOperation m_eOperation; // Working kind
};