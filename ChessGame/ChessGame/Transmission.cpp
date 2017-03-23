#include "Transmission.h"


Transmission::Transmission()
{
	// Transmission variables
	m_sock = INVALID_SOCKET;
	ZeroMemory(&m_serveraddr, sizeof(m_serveraddr));
}

Transmission::~Transmission() { WSACleanup(); }

void Transmission::SetServerIP(const HWND& hIpEdit)
{
	GetWindowText(hIpEdit, m_serverip, MAX_IP_LEN);
}

bool Transmission::Init(const HWND& hWnd)
{
	// Winsock initialize
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		printf("[Error] Location : Transmission::Init, Reason : WSAStartup() has been failed: %d \n", WSAGetLastError());
		return false;
	}

	// socket()
	m_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (m_sock == INVALID_SOCKET) {
		printf("[Error] Location : Transmission::Init, Reason : socket() has been failed: %d \n", WSAGetLastError());
		return false;
	}

	// WSAAsyncSelect()
	if (WSAAsyncSelect(m_sock, hWnd, WM_SOCKET, FD_CONNECT | FD_WRITE | FD_CLOSE) == SOCKET_ERROR) {
	printf("[Error] Location : Transmission::Init, Reason : (WSAAsyncSelect() has been failed: %d \n", WSAGetLastError());
	return false;
	}

	// bind()
	m_serveraddr.sin_family = AF_INET;
	m_serveraddr.sin_addr.s_addr = inet_addr(m_serverip);
	m_serveraddr.sin_port = htons(SERVER_PORT);
	m_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_sock == INVALID_SOCKET) {
		printf("[Error] Location : Transmission::Init, Reason : socket() has been failed: %d \n", WSAGetLastError());
		return false;
	}

	return true;
}

void Transmission::ProcessSocketMessage(const HWND& hWnd, const UINT& iMessage, const WPARAM& wParam, const LPARAM& lParam)
{
	
	switch (WSAGETSELECTEVENT(lParam))
	{
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
	if (WSAAsyncSelect(m_sock, hWnd, WM_SOCKET, FD_READ | FD_CLOSE) == SOCKET_ERROR) {
		printf("[Error] Location : Transmission::ProcessSocketMessage, Reason : (WSAAsyncSelect() has been failed: %d \n", WSAGetLastError());
		return;
	}
}

bool Transmission::Connect()
{
		int retval{};
		retval = connect(m_sock, (SOCKADDR*)&m_serveraddr, sizeof(m_serveraddr));
		if (SOCKET_ERROR == retval) {
			printf("[Error] Location : Transmission::Connect, Reason : connect() has been failed: %d \n", WSAGetLastError());
			return false;
		}
		return true;
		
		bool t = false;
	
}

void Transmission::Close(bool forceclose)
{
	struct linger stLinger = { 0, 0 };
	// Is closed by force?
	if (true == forceclose)
		stLinger.l_onoff = 1;
	// Stop send and recv 
	shutdown(m_sock, SD_BOTH);
	// Set the option
	setsockopt(m_sock, SOL_SOCKET, SO_LINGER, (char*)&stLinger, sizeof(stLinger));
	// Close Connection
	closesocket(m_sock);
	m_sock = INVALID_SOCKET;
}