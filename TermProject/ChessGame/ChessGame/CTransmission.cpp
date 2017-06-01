#include "CTransmission.h"


CTransmission::CTransmission()
{
	// CTransmission variables
	m_sock = INVALID_SOCKET;
	ZeroMemory(&m_saServerAddr, sizeof(m_saServerAddr));
	m_nClientCnt = 0;

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
	m_hWnd = a_hWnd;
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
	case SIGNUP::eSC_SIGNUP_FAIL:
		MessageBox(NULL, "Sign up has been failed. Plz input another ID.", "Sign up Fail", MB_OK | MB_ICONERROR); break;
	case SIGNUP::eSC_SIGNUP_SUCCESS:
		MessageBox(NULL, "Sign up success. Have fun!", "Sign up Success", NULL); EndDialog(m_hSignUpDlg, 0); break;
	case LOGIN::eSC_LOGIN_FAIL_INCORRECT:
		MessageBox(NULL, "ID or PWD dosen't correct.", "Login Fail", MB_OK | MB_ICONERROR); break;
	case LOGIN::eSC_LOGIN_FAIL_LOGINED:
		MessageBox(NULL, "UR ID is already logined.", "Login Fail", MB_OK | MB_ICONERROR); break;
	case LOGIN::eSC_LOGIN_SUCCESS: {
		ST_SC_LOGIN_SUCCESS* stPacket = reinterpret_cast<ST_SC_LOGIN_SUCCESS*>(a_ptr);
		memcpy(&m_DBInfo, &stPacket->m_DBInfo,sizeof(stPacket->m_DBInfo));
		EndDialog(m_hDlg, 0);
		ShowWindow(m_PlayerInfoDlg, SW_SHOW);
		SetFocus(m_hWnd);
		this->ShowPlayerInfo();
		SetTimer(m_hWnd, 1, 1000 / 60, NULL);
		break;
	} 
	case CHAT::eSC_CHAT: {
		ST_SC_CHAT* stPacket = reinterpret_cast<ST_SC_CHAT*>(a_ptr);
		SendMessage(m_hList, LB_ADDSTRING, 0, (LPARAM)stPacket->m_Message);
		break;
	}
	case eSC_PUT_CLIENT: {
		ST_SC_PUT_OBJECT* stPacket = reinterpret_cast<ST_SC_PUT_OBJECT*>(a_ptr);
		for (int i = 0; i < MAX_PLAYER; ++i) {
			if (i == stPacket->m_wId && m_First) {
				m_wId = stPacket->m_wId;
				m_DBInfo.m_pos = stPacket->m_pos;
				m_First = false;

				m_stClientInfo[i].m_IsConnected = true;
				m_stClientInfo[i].m_wId = stPacket->m_wId;
				m_stClientInfo[i].m_Info.m_pos = stPacket->m_pos;
				m_nClientCnt++;
				break;
			}

			else if (i == stPacket->m_wId && i != m_wId) {
				m_stClientInfo[i].m_IsConnected = true;
				m_stClientInfo[i].m_wId = stPacket->m_wId;
				m_stClientInfo[i].m_Info.m_pos = stPacket->m_pos;
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
					m_DBInfo.m_pos = stPacket->m_pos;
					m_stClientInfo[i].m_Info.m_pos = stPacket->m_pos;
					this->UpdateZoneInfo();
					break;
				}
				else {
					m_stClientInfo[i].m_Info.m_pos = stPacket->m_pos;
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

const Point& CTransmission::GetPos(){ return m_DBInfo.m_pos; }

void CTransmission::SendLoginPacket(char a_ID[], char a_PWD[])
{
	ST_CS_LOGIN stPacket;
	stPacket.m_bytSize = sizeof(stPacket);
	stPacket.m_bytType = eCS_LOGIN;
	stPacket.m_bytIDLen = strlen(a_ID);
	stPacket.m_bytPWDLen = strlen(a_PWD);
	memcpy(stPacket.m_ID, a_ID, strlen(a_ID));
	memcpy(stPacket.m_PWD, a_PWD, strlen(a_PWD));
	memcpy(m_send_wsabuf.buf, &stPacket, sizeof(stPacket));

	m_send_wsabuf.len = sizeof(stPacket);


	DWORD iobyte;
	WSASend(m_sock, &m_send_wsabuf, 1, &iobyte, 0, NULL, NULL);

}

void CTransmission::SendSignUpPacket(char a_ID[], char a_PWD[])
{
	ST_CS_LOGIN stPacket;
	stPacket.m_bytSize = sizeof(stPacket);
	stPacket.m_bytType = eCS_SIGNUP;
	stPacket.m_bytIDLen = strlen(a_ID);
	stPacket.m_bytPWDLen = strlen(a_PWD);
	memcpy(stPacket.m_ID, a_ID, strlen(a_ID));
	memcpy(stPacket.m_PWD, a_PWD, strlen(a_PWD));
	memcpy(m_send_wsabuf.buf, &stPacket, sizeof(stPacket));

	m_send_wsabuf.len = sizeof(stPacket);


	DWORD iobyte;
	WSASend(m_sock, &m_send_wsabuf, 1, &iobyte, 0, NULL, NULL);

}

void CTransmission::SendChatPacket(char a_Message[])	
{
	ST_CS_CHAT stPacket;
	stPacket.m_bytSize = sizeof(stPacket);
	stPacket.m_bytType = eCS_CHAT;

	memcpy(stPacket.m_Message, a_Message, strlen(a_Message));
	stPacket.m_bytMessageLen = strlen(a_Message);
	memcpy(m_send_wsabuf.buf, &stPacket, sizeof(stPacket));

	m_send_wsabuf.len = sizeof(stPacket);

	DWORD iobyte;
	WSASend(m_sock, &m_send_wsabuf, 1, &iobyte, 0, NULL, NULL);

}

void CTransmission::UpdateZoneInfo()
{
	char str[4];
	ZeroMemory(str, sizeof(str)); itoa(m_DBInfo.m_pos.m_wZone, str, 10); SetDlgItemText(m_PlayerInfoDlg, IDC_ZONE, str);
}

void CTransmission::ShowPlayerInfo()
{
	char str[4];

	SetDlgItemText(m_PlayerInfoDlg, IDC_ID, m_DBInfo.ID); ZeroMemory(str, sizeof(str)); itoa(m_DBInfo.m_Level, str,10);
	SetDlgItemText(m_PlayerInfoDlg, IDC_Level,str); ZeroMemory(str, sizeof(str)); itoa(m_DBInfo.m_MAX_HP, str, 10);
	SetDlgItemText(m_PlayerInfoDlg, IDC_MAX_HP, str); ZeroMemory(str, sizeof(str)); itoa(m_DBInfo.m_HP, str, 10);
	SetDlgItemText(m_PlayerInfoDlg, IDC_HP, str);  ZeroMemory(str, sizeof(str)); itoa(m_DBInfo.m_MAX_EXP, str, 10);
	SetDlgItemText(m_PlayerInfoDlg, IDC_MAX_EXP, str);  ZeroMemory(str, sizeof(str)); itoa(m_DBInfo.m_EXP, str, 10);
	SetDlgItemText(m_PlayerInfoDlg, IDC_EXP, str);  ZeroMemory(str, sizeof(str)); itoa(m_DBInfo.m_Damage, str, 10);
	SetDlgItemText(m_PlayerInfoDlg, IDC_DMG, str);  ZeroMemory(str, sizeof(str)); itoa(m_DBInfo.m_pos.m_wZone, str, 10);
	SetDlgItemText(m_PlayerInfoDlg, IDC_ZONE, str);
}