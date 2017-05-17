#include "CTransmission.h"


CTransmission::CTransmission()
{
	// CTransmission variables
	m_sock = INVALID_SOCKET;
	ZeroMemory(&m_saServerAddr, sizeof(m_saServerAddr));
	m_nClientCnt = 0;

	m_pos.m_wX = FIRST_X;
	m_pos.m_wY = FIRST_Y;
	m_pos.m_wZone = 1;
	m_wId = 65535;
	m_IsMoved = false;
	m_First = true;
	m_nPacketType = -1;
	
	m_send_wsabuf.buf = m_send_buffer;
	m_send_wsabuf.len = MAX_BUF_SIZE;
	m_recv_wsabuf.buf = m_recv_buffer;
	m_recv_wsabuf.len = MAX_BUF_SIZE;

	m_NightImg.Load("Night.png");
	m_PawnImg.Load("Pawn.png");

	
}

CTransmission::~CTransmission() { WSACleanup(); }

void CTransmission::SetServerIP(const HWND& a_hIpEdit)
{
	GetWindowText(a_hIpEdit, m_tchServerIp, MAX_IP_LEN);
}

const bool& CTransmission::Init(const HWND& a_hWnd)
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



void CTransmission::ReadPacket()
{
	DWORD iobyte, ioflag = 0;

	int ret = WSARecv(m_sock, &m_recv_wsabuf, 1, &iobyte, &ioflag, NULL, NULL);
	if (ret) {
		int err_code = WSAGetLastError();
		printf("Recv Error [%d]\n", err_code);
	}

	BYTE *ptr = reinterpret_cast<BYTE *>(m_recv_buffer);

	while (0 != iobyte) {
		if (0 == m_in_packet_size) m_in_packet_size = ptr[0];
		if (iobyte + m_saved_packet_size >= m_in_packet_size) {
			memcpy(m_packet_buffer + m_saved_packet_size, ptr, m_in_packet_size - m_saved_packet_size);
			ProcessPacket(m_packet_buffer);
			ptr += m_in_packet_size - m_saved_packet_size;
			iobyte -= m_in_packet_size - m_saved_packet_size;
			m_in_packet_size = 0;
			m_saved_packet_size = 0;
		}
		else {
			memcpy(m_packet_buffer + m_saved_packet_size, ptr, iobyte);
			m_saved_packet_size += iobyte;
			iobyte = 0;
		}
	}

}


const bool& CTransmission::ProcessPacket(const HWND& a_hWnd, const UINT& a_iMessage, const WPARAM& a_wParam, const LPARAM& a_lParam)
{
	switch (WSAGETSELECTEVENT(a_lParam)){
	case FD_READ:
		ReadPacket();
		return true;
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

void CTransmission::ProcessPacket(char* a_ptr)
{

	switch (a_ptr[1]) {
	case eSC_PUT_CLIENT: {
		ST_SC_PUT_OBJECT* stPacket = reinterpret_cast<ST_SC_PUT_OBJECT*>(a_ptr);
		for (int i = 0; i < MAX_PLAYER; ++i) {
			if (i == stPacket->m_wId && m_First) {
				m_wId = stPacket->m_wId;
				m_pos = stPacket->m_pos;
				m_First = false;

				m_stClientInfo[i].m_IsConnected = true;
				m_stClientInfo[i].m_wId = stPacket->m_wId;
				m_stClientInfo[i].m_pos = stPacket->m_pos;
				m_nClientCnt++;
				break;
			}

			else if (i == stPacket->m_wId && i != m_wId) {
				m_stClientInfo[i].m_IsConnected = true;
				m_stClientInfo[i].m_wId = stPacket->m_wId;
				m_stClientInfo[i].m_pos = stPacket->m_pos;
				m_nClientCnt++;
				break;

			}
		}
		break;
	}

	case eSC_MOVE_CLIENT: {
		ST_SC_MOVE_OBJECT* stPacket = reinterpret_cast<ST_SC_MOVE_OBJECT*>(a_ptr);
		for (int i = 0; i < MAX_PLAYER; ++i) {
			if (m_stClientInfo[i].m_wId == stPacket->m_wId) {
				if (m_stClientInfo[i].m_wId == m_wId) {
					m_pos = stPacket->m_pos;
					m_stClientInfo[i].m_pos = stPacket->m_pos;
					break;
				}
				else {
					m_stClientInfo[i].m_pos = stPacket->m_pos;
					break;
				}
			}
		}
		break;
	}
	case eSC_REMOVE_CLIENT: {
		ST_SC_REMOVE_OBJECT* stPacket = reinterpret_cast<ST_SC_REMOVE_OBJECT*>(a_ptr);
		for (int i = 0; i < MAX_PLAYER; ++i) {
			if (i == stPacket->m_wId) { m_stClientInfo[i].m_IsConnected = false; break; }
		}
		break;
	}

	case eSC_PUT_NPC: {
		ST_SC_PUT_OBJECT* stPacket = reinterpret_cast<ST_SC_PUT_OBJECT*>(a_ptr);
		m_stNPCInfo[stPacket->m_wId].m_wId = stPacket->m_wId;
		m_stNPCInfo[stPacket->m_wId].m_pos = stPacket->m_pos;
		m_stNPCInfo[stPacket->m_wId].m_IsAlive = true;
		break;
	}

	case eSC_MOVE_NPC: {
		ST_SC_MOVE_OBJECT* stPacket = reinterpret_cast<ST_SC_MOVE_OBJECT*>(a_ptr);
		for (WORD i = 0; i < MAX_NPC_NUM; ++i) {
			if (stPacket->m_wId == i) {
				m_stNPCInfo[i].m_pos = stPacket->m_pos;
			}
		}

		break;
	}
	case eSC_REMOVE_NPC: {
		ST_SC_REMOVE_OBJECT* stPacket = reinterpret_cast<ST_SC_REMOVE_OBJECT*>(a_ptr);
		m_stNPCInfo[stPacket->m_wId].m_IsAlive = false;
		break;
	}
	case eSC_MAP_NOTIFY: {
		ST_SC_NOTIFY_MAP* stPacket = reinterpret_cast<ST_SC_NOTIFY_MAP*>(a_ptr);
		m_stMap.m_MapInfo.push_back(stPacket->m_pos);
		break;
	}

	}
}

const bool& CTransmission::Connect(const HWND& a_hWnd)
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




void CTransmission::SendPacket()
{
	ST_CS_MOVE* stPacket = reinterpret_cast<ST_CS_MOVE*>(m_send_buffer);
	stPacket->m_bytSize = sizeof(stPacket);
	m_send_wsabuf.len = sizeof(stPacket);

	DWORD iobyte;

	switch (m_nPacketType) {
	case eCS_UP:
		stPacket->m_bytType = eCS_UP;
		WSASend(m_sock, &m_send_wsabuf, 1, &iobyte, 0, NULL, NULL);
		break;
	case eCS_DOWN:
		stPacket->m_bytType = eCS_DOWN;
		WSASend(m_sock, &m_send_wsabuf, 1, &iobyte, 0, NULL, NULL);
		break;
	case eCS_RIGHT:
		stPacket->m_bytType = eCS_RIGHT;
		WSASend(m_sock, &m_send_wsabuf, 1, &iobyte, 0, NULL, NULL);
		break;
	case eCS_LEFT:
		stPacket->m_bytType = eCS_LEFT;
		WSASend(m_sock, &m_send_wsabuf, 1, &iobyte, 0, NULL, NULL);
		break;

	}
}



void CTransmission::Close(const bool& a_bForceClose)
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

const Point& CTransmission::GetPos(){ return m_pos; }