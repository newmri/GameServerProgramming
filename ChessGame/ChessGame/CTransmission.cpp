#include "CTransmission.h"


CTransmission::CTransmission()
{
	// CTransmission variables
	m_sock = INVALID_SOCKET;
	ZeroMemory(&m_saServerAddr, sizeof(m_saServerAddr));
}

CTransmission::~CTransmission() { WSACleanup(); }

void CTransmission::SetServerIP(const HWND& a_hIpEdit)
{
	GetWindowText(a_hIpEdit, m_tchServerIp, MAX_IP_LEN);
}

bool CTransmission::Init(const HWND& a_hWnd)
{
	// Winsock initialize
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		printf("[Error] Location : CTransmission::Init, Reason : WSAStartup() has been failed: %d \n", WSAGetLastError());
		return false;
	}

	// socket()
	m_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (m_sock == INVALID_SOCKET) {
		printf("[Error] Location : CTransmission::Init, Reason : socket() has been failed: %d \n", WSAGetLastError());
		return false;
	}

	// WSAAsyncSelect()
	if (WSAAsyncSelect(m_sock, a_hWnd, WM_SOCKET, FD_CONNECT | FD_WRITE | FD_CLOSE) == SOCKET_ERROR) {
	printf("[Error] Location : CTransmission::Init, Reason : WSAAsyncSelect() has been failed: %d \n", WSAGetLastError());
	return false;
	}

	// bind()
	m_saServerAddr.sin_family = AF_INET;
	m_saServerAddr.sin_addr.s_addr = inet_addr(m_tchServerIp);
	m_saServerAddr.sin_port = htons(SERVER_PORT);
	m_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_sock == INVALID_SOCKET) {
		printf("[Error] Location : CTransmission::Init, Reason : socket() has been failed: %d \n", WSAGetLastError());
		return false;
	}

	return true;
}

void CTransmission::ProcessSocketMessage(const HWND& a_hWnd, const UINT& a_iMessage, const WPARAM& a_wParam, const LPARAM& a_lParam)
{
	switch (WSAGETSELECTEVENT(a_lParam)){
	case FD_READ:
		printf("ddd");
		break;
	case FD_CLOSE:
		Close(true);
		return;
	default:
		break;
	}

	// WSAAsyncSelect()
	if (WSAAsyncSelect(m_sock, a_hWnd, WM_SOCKET, FD_READ | FD_CLOSE) == SOCKET_ERROR) {
		printf("[Error] Location : CTransmission::ProcessSocketMessage, Reason : (WSAAsyncSelect() has been failed: %d \n", WSAGetLastError());
		return;
	}
}

bool CTransmission::Connect()
{
		int retval{};
		retval = connect(m_sock, (SOCKADDR*)&m_saServerAddr, sizeof(m_saServerAddr));
		if (SOCKET_ERROR == retval) {
			printf("[Error] Location : CTransmission::Connect, Reason : connect() has been failed: %d \n", WSAGetLastError());
			return false;
		}
		return true;
	
}

void CTransmission::Close(bool a_bForceClose)
{
	struct linger stLinger = { 0, 0 };
	// Is closed by force?
	if (true == a_bForceClose)
		stLinger.l_onoff = 1;
	// Stop send and recv 
	shutdown(m_sock, SD_BOTH);
	// Set the option
	setsockopt(m_sock, SOL_SOCKET, SO_LINGER, (char*)&stLinger, sizeof(stLinger));
	// Close Connection
	closesocket(m_sock);
	m_sock = INVALID_SOCKET;
}