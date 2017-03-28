#include "CTransmission.h"


CTransmission::CTransmission()
{
	// CTransmission variables
	m_sock = INVALID_SOCKET;
	ZeroMemory(&m_saServerAddr, sizeof(m_saServerAddr));
	m_pos.x = 0, m_pos.y = 0;
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

POINT CTransmission::GetPos(){ return m_pos; }

bool CTransmission::ProcessSocketMessage(const HWND& a_hWnd, const UINT& a_iMessage, const WPARAM& a_wParam, const LPARAM& a_lParam)
{
	switch (WSAGETSELECTEVENT(a_lParam)){
	case FD_READ:
		Recv(m_szBuf,MAX_BUF_SIZE, 0);

		switch(m_szBuf[sizeof(int)]) {
		case eMOVE: {
			// pos x
			m_pos.x = atoi(&m_szBuf[sizeof(int) * 2]);
			// pos y
			m_pos.y = atoi(&m_szBuf[sizeof(int) * 3]);
			return true;
		}
		case eCLIENT_INFO: {
			for (int i = 0; i < MAX_PLAYER; ++i) {
				if (m_stPlayerInfo[i].m_usId == 0) {
					memcpy(&m_stPlayerInfo[i], &m_szBuf[sizeof(int) * 2], sizeof(stPlayerInfo));
					printf("%d", m_stPlayerInfo[i].m_pos.y);
					break;
				}

			}
		}
		}
	case FD_CLOSE:
		Close(true);
		return true;
	default:
		break;
	}

	// WSAAsyncSelect()
	if (WSAAsyncSelect(m_sock, a_hWnd, WM_SOCKET, FD_READ | FD_CLOSE) == SOCKET_ERROR) {
		printf("[Error] Location : CTransmission::ProcessSocketMessage, Reason : (WSAAsyncSelect() has been failed: %d \n", WSAGetLastError());
		return false;
	}
}

bool CTransmission::Connect(const HWND& a_hWnd)
{
	int nRetval{};
	nRetval = connect(m_sock, (SOCKADDR*)&m_saServerAddr, sizeof(m_saServerAddr));
	if (nRetval == SOCKET_ERROR) {
		printf("[Error] Location : CTransmission::Connect, Reason : connect() has been failed: %d \n", WSAGetLastError());
		return false;
	}
	// WSAAsyncSelect()
	if (WSAAsyncSelect(m_sock, a_hWnd, WM_SOCKET, FD_CONNECT | FD_READ | FD_CLOSE) == SOCKET_ERROR) {
		printf("[Error] Location : CTransmission::Init, Reason : WSAAsyncSelect() has been failed: %d \n", WSAGetLastError());
		return false;
	}
	return true;
}

bool CTransmission::Recv(char* data, int len, int flags)
{
	int nRetval{};

	// nReceived data(flexible)
	nRetval = Recvn(data, len, flags);
	if (nRetval == SOCKET_ERROR){
		printf("[Error] Location : CTransmission::Recv, Reason : Recvn() has been failed \n");
		return false;
	}

	else if (nRetval == 0) {
		printf("[Error] Location : CTransmission::Recv, Reason : disconnected \n");
		return false;
	}

	return true;
}

int CTransmission::Recvn(char* buf, int len, int flags)
{
	int nReceived;
	char* chptr = buf;
	int nLeft{};

	nReceived = recv(m_sock, chptr, sizeof(int), flags);
	nLeft = *chptr;
	chptr += nReceived;

	while (nLeft > 0){
		nReceived = recv(m_sock, chptr, nLeft, flags);
		if (nReceived == SOCKET_ERROR) return SOCKET_ERROR;
		else if (nReceived == 0) break;
		nLeft -= nReceived;
		chptr += nReceived;
	}
	return(len - nLeft);
}

void CTransmission::AssembleAndSendPacket(enumDataType& a_eDataType)
{
	switch (a_eDataType) {
	case eMOVE: {
		int nCommand = eMOVE;
		m_nDataLen = POS_LEN;
		// len
		memcpy(m_szBuf, &m_nDataLen, sizeof(m_nDataLen));
		// command
		memcpy(&m_szBuf[sizeof(m_nDataLen)], &nCommand, sizeof(int));
		// pos x
		itoa(m_pos.x, &m_szBuf[sizeof(m_nDataLen) + sizeof(nCommand)], 10);
		// pos y
		itoa(m_pos.y, &m_szBuf[sizeof(m_nDataLen) + sizeof(nCommand) + sizeof(int)], 10);
		m_nDataLen += sizeof(int);
		Send(m_szBuf, m_nDataLen);
		break;
	}

	}
}

bool CTransmission::Send(char* data, int len)
{
	int nRetval{};
	// sending data(flexible)
	nRetval = send(m_sock, data, len, 0);
	if (SOCKET_ERROR == nRetval) {
		printf("[Error] Location : CTransmission::Send, Reason :  send() has been failed \n");
		return false;
	}

	else if (nRetval == 0) {
		printf("[Error] Location : CTransmission::Send, Reason : disconnected \n");
		return false;
	}
	return true;
}

void CTransmission::Close(bool a_bForceClose)
{
	struct linger stLinger = { 0, 0 };
	// Is closed by force?
	if (a_bForceClose == true)
		stLinger.l_onoff = 1;
	// Stop send and recv 
	shutdown(m_sock, SD_BOTH);
	// Set the option
	setsockopt(m_sock, SOL_SOCKET, SO_LINGER, (char*)&stLinger, sizeof(stLinger));
	// Close Connection
	closesocket(m_sock);
	m_sock = INVALID_SOCKET;
}