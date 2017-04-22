#pragma once

#include "Define.h"

// WSAOVERLAPPED Expansion of structure 
struct stOverlappedEx {
	WSAOVERLAPPED m_wsaOverlapped; // Overlapped I/O Structure
	WSABUF m_wsaBuf; // Overlapped I/O Working buffer
	unsigned char m_szBuf[MAX_BUF_SIZE]; // Data buffer
	enumOperation m_eOperation; // Working kind
};