#include "IOCP.h"

IOCP::IOCP()
{
	// Initialize all variables
	m_bWorkerRun = true;
	m_bAccepterRun = true;
	m_nClientCnt = 0;
	m_hAccepterThread = NULL;
	m_hIOCP = NULL;
	m_SocketListen = INVALID_SOCKET;
	m_retval = false;
	ZeroMemory(m_szSocketBuf, MAX_BUF_SIZE);
	for (int i = 0; i < MAX_WORKER_THREAD; i++)
		m_hWorkerThread[i] = NULL;

	// Contruct structures that have client's information 
	m_pClientInfo = new stClientInfo[MAX_CLIENT_NUM];
}

IOCP::~IOCP(void)
{
	WSACleanup();

	// Delete used obejcts
	if (m_pClientInfo) {
		delete[] m_pClientInfo;
		m_pClientInfo = NULL;
	}
}

bool IOCP::InitSocket()
{
	WSADATA wsaData;
	// Initialized winsock version in 2.2 
	int nRet = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (0 != nRet) {
		cout<<"[Error] WSAStartup() has been failed: " << WSAGetLastError() << endl;
		return false;
	}
		
	// Construct TCP Overlapped I/O Socket
	m_SocketListen = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, NULL, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == m_SocketListen) {
		cout << "[Error] WSASocket() has been failed: " << WSAGetLastError() << endl;
		return false;
	}

	cout << "Initializing socket has been succeeded" << endl;
	return true;
}


void IOCP::CloseSocket(stClientInfo* pClientInfo, bool bIsForce)
{
	struct linger stLinger = { 0,0 }; // SO_DONTLINGER

	// Is closed by force?									  
	if (true == bIsForce)
		stLinger.l_onoff = 1;

	// Stop send and recv 
	shutdown(pClientInfo->m_SocketClient, SD_BOTH);
	// Set the option
	setsockopt(pClientInfo->m_SocketClient, SOL_SOCKET, SO_LINGER, (char*)&stLinger, sizeof(stLinger));
	// Close Connection
	closesocket(pClientInfo->m_SocketClient);
	pClientInfo->m_SocketClient = INVALID_SOCKET;
}