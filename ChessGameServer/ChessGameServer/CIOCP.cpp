#include "CIOCP.h"
#include <process.h>

// to proccess WSARecv and WSASend 
unsigned int  WINAPI CallWorkerThread(LPVOID a_p)
{
	CIOCP* pcOverlappedEvent = (CIOCP*)a_p;
	pcOverlappedEvent->WorkerThread();
	return 0;
}

unsigned int WINAPI CallAccepterThread(LPVOID a_p)
{
	CIOCP* pcOverlappedEvent = (CIOCP*)a_p;
	pcOverlappedEvent->AccepterThread();
	return 0;
}

CIOCP::CIOCP()
{
	// Initialize all variables
	m_bWorkerRun = true;
	m_bAccepterRun = true;
	m_nClientCnt = 0;
	m_hAccepterThread = NULL;
	m_hIOCP = NULL;
	m_ListenSocket = INVALID_SOCKET;
	m_bRetval = false;
	ZeroMemory(m_szSocketBuf, MAX_BUF_SIZE);
	for (int i = 0; i < MAX_WORKER_THREAD; i++)
		m_hWorkerThread[i] = NULL;

	// Contruct structures that have client's information 
	m_stpClientInfo = new stClientInfo[MAX_CLIENT_NUM];
	m_pos.x = CHESS_FIRST_X;
	m_pos.y = CHESS_FIRST_Y;
}

CIOCP::~CIOCP(void)
{
	WSACleanup();

	// Delete used obejcts
	if (m_stpClientInfo) {
		delete[] m_stpClientInfo;
		m_stpClientInfo = NULL;
	}
}

bool CIOCP::InitSocket()
{
	WSADATA wsaData;
	// Initialized winsock version in 2.2 
	int nRet = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (0 != nRet) {
		printf("[Error] Location : CIOCP::InitSocket, Reason :WSAStartup() has been failed: %d \n", WSAGetLastError());
		return false;
	}
		
	// Construct TCP Overlapped I/O Socket
	m_ListenSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, NULL, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == m_ListenSocket) {
		printf("[Error] Location : CIOCP::InitSocket, Reason :WSASocket() has been failed: %d \n", WSAGetLastError());
		return false;
	}

	cout << "Initializing socket has been succeeded" << endl;
	return true;
}


void CIOCP::CloseSocket(stClientInfo* a_a_a_pClientInfo, bool a_bIsForce)
{
	struct linger stLinger = { 0,0 }; // SO_DONTLINGER

	// Is closed by force?									  
	if (true == a_bIsForce)
		stLinger.l_onoff = 1;

	// Stop send and recv 
	shutdown(a_a_a_pClientInfo->m_SocketClient, SD_BOTH);
	// Set the option
	setsockopt(a_a_a_pClientInfo->m_SocketClient, SOL_SOCKET, SO_LINGER, (char*)&stLinger, sizeof(stLinger));
	// Close Connection
	closesocket(a_a_a_pClientInfo->m_SocketClient);
	a_a_a_pClientInfo->m_SocketClient = INVALID_SOCKET;
}

bool CIOCP::BindandListen(int a_nBindPort)
{
	SOCKADDR_IN stServerAddr;
	stServerAddr.sin_family = AF_INET;
	stServerAddr.sin_port = htons(a_nBindPort);
	stServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	// bind
	int nRet = bind(m_ListenSocket, (SOCKADDR*)&stServerAddr, sizeof(SOCKADDR_IN));
	if (0 != nRet) {
		printf("[Error] Location : CIOCP::BindandListen, Reason : bind() has been failed: %d \n", WSAGetLastError());
		return false;
	}

	nRet = listen(m_ListenSocket, 5);
	if (0 != nRet) {
		printf("[Error] Location : CIOCP::BindandListen, Reason : listen() has been failed: %d \n", WSAGetLastError());
		return false;
	}
	cout << "Enrollment Success!" << endl;
	return true;
}

bool CIOCP::CreateWorkerThread()
{
	unsigned int uiThreadId = 0;

	// Thread Creation to input into WaitingThread Queue
	for (int i = 0; i < MAX_WORKER_THREAD; i++) {
		m_hWorkerThread[i] = (HANDLE)_beginthreadex(NULL, 0, &CallWorkerThread, this, CREATE_SUSPENDED, &uiThreadId);
		if (m_hWorkerThread == NULL) {
			printf("[Error] Location : CIOCP::CreateWorkerThread, Reason : CallWorkerThread() has been failed: %d \n", WSAGetLastError());
			return false;
		}
		ResumeThread(m_hWorkerThread[i]);
	}
	cout << "Worker thread is starting!" << endl;
	return true;
}

bool CIOCP::CreateAccepterThread()
{
	unsigned int uiThreadId = 0;
	m_hAccepterThread = (HANDLE)_beginthreadex(NULL, 0, &CallAccepterThread, this, CREATE_SUSPENDED, &uiThreadId);
	if (m_hAccepterThread == NULL) {
		printf("[Error] Location : CIOCP::CreateAccepterThread, Reason : CallAccepterThread() has been failed: %d \n", WSAGetLastError());
		return false;
	}
	ResumeThread(m_hAccepterThread);
	cout << "Accept thread is starting!" << endl;
	return true;
}

bool CIOCP::BindIOCompletionPort(stClientInfo* a_pClientInfo)
{
	HANDLE hIOCP;
	// Connect socket and pClientInfo into COmpletionPort object
	hIOCP = CreateIoCompletionPort((HANDLE)a_pClientInfo->m_SocketClient, m_hIOCP, reinterpret_cast<DWORD>(a_pClientInfo), 0);
	if (hIOCP == NULL || m_hIOCP != hIOCP) {
		printf("[Error] Location : CIOCP::BindIOCompletionPort, Reason : CreateIoCompletionPort() has been failed: %d \n", WSAGetLastError());
		return false;
	}
	return true;
}

void CIOCP::StartServer()
{
	// Create CompletionPort Object
	m_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
	if (m_hIOCP == NULL) {
		printf("[Error] Location : CIOCP::StartServer, Reason : CreateIoCompletionPort() has been failed: %d \n", WSAGetLastError());
		return;
	}
	cout << "Server is started!" << endl;
	bool bRet = CreateWorkerThread();
	if (bRet == false) return;

	SOCKADDR_IN stClientAddr;
	int nAddrLen = sizeof(SOCKADDR_IN);
	while (true) {
		// Get index
		stClientInfo* pClientInfo = GetEmptyClientInfo();
		if (pClientInfo == NULL) {
			printf("[Error] Location : CIOCP::AccepterThread, Reason : Num of client is full \n");
			return;
		}
		// Wait for connection
		pClientInfo->m_SocketClient = accept(m_ListenSocket, (SOCKADDR*)&stClientAddr, &nAddrLen);
		if (pClientInfo->m_SocketClient == INVALID_SOCKET) continue;
		// Store Address Information
		memcpy(&pClientInfo->m_saClientAddr, &stClientAddr, sizeof(SOCKADDR_IN));
		// Connect I/O Completion Port and socket 
		bool bRet = BindIOCompletionPort(pClientInfo);
		if (bRet == false) return;
		// Request Recv Overlapped I/O 
		bRet = BindRecv(pClientInfo);
		if (bRet == false) return;
		printf("A cllient has been connected: IP(%s) SOCKET(%d) \n", inet_ntoa(stClientAddr.sin_addr), pClientInfo->m_SocketClient);
		
		pClientInfo->m_eLocation = eGAME_ROOM;
		char buf[MAX_BUF_SIZE];

		int command = eMOVE;
		int len = (sizeof(POINT)) + sizeof(int);
		memcpy(buf, &len, sizeof(int));
		memcpy(&buf[sizeof(int)], &command, sizeof(int));
		itoa(m_pos.x, &buf[sizeof(int) * 2], 10);
		itoa(m_pos.y, &buf[sizeof(int) * 3], 10);
		len += sizeof(int);
		SendMsg(pClientInfo, buf, len);

		m_nClientCnt++;
	}

}

bool CIOCP::BindRecv(stClientInfo* a_a_pClientInfo)
{
	DWORD dwFlag = 0;
	DWORD dwRecvNumBytes = 0;

	// Set Overlapped I/O Information
	a_a_pClientInfo->m_stRecvOverlappedEx.m_wsaBuf.len = MAX_BUF_SIZE;
	a_a_pClientInfo->m_stRecvOverlappedEx.m_wsaBuf.buf = a_a_pClientInfo->m_stRecvOverlappedEx.m_szBuf;
	a_a_pClientInfo->m_stRecvOverlappedEx.m_eOperation = eOP_RECV;

	int nRet = WSARecv(a_a_pClientInfo->m_SocketClient, &(a_a_pClientInfo->m_stRecvOverlappedEx.m_wsaBuf), 1, &dwRecvNumBytes, &dwFlag, (LPWSAOVERLAPPED)&(a_a_pClientInfo->m_stRecvOverlappedEx), NULL);

	// A Client is disconnected
	if (nRet == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING)) {
		printf("[Error] Location : CIOCP::BindRecv, Reason : WSARecv() has been failed: %d \n", WSAGetLastError());
		return false;
	}
	return true;
}

bool CIOCP::SendMsg(stClientInfo* a_pClientInfo, char* a_pMsg, int a_nLen)
{
	DWORD dwRecvNumBytes = 0;

	// Copy message to send
	CopyMemory(a_pClientInfo->m_stSendOverlappedEx.m_szBuf, a_pMsg, a_nLen);

	// Set Overlapped I/O Information
	a_pClientInfo->m_stRecvOverlappedEx.m_wsaBuf.len = a_nLen;
	a_pClientInfo->m_stRecvOverlappedEx.m_wsaBuf.buf = a_pClientInfo->m_stSendOverlappedEx.m_szBuf;
	a_pClientInfo->m_stRecvOverlappedEx.m_eOperation = eOP_SEND;


	int nRet = WSASend(a_pClientInfo->m_SocketClient, &(a_pClientInfo->m_stRecvOverlappedEx.m_wsaBuf), 1, &dwRecvNumBytes, 0, (LPWSAOVERLAPPED)&(a_pClientInfo->m_stRecvOverlappedEx), NULL);
	
	if (nRet == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING)) {
		printf("[Error] Location : CIOCP::SendMsg, Reason : WSASend() has been failed: %d \n", WSAGetLastError());
		return false;
	}
	return true;
}

stClientInfo* CIOCP::GetEmptyClientInfo()
{
	for (int i = 0; i < MAX_CLIENT_NUM; i++) {
		if (INVALID_SOCKET == m_stpClientInfo[i].m_SocketClient) return &m_stpClientInfo[i];
	}
	return NULL;
}

void CIOCP::AccepterThread()
{
	SOCKADDR_IN stClientAddr;
	int nAddrLen = sizeof(SOCKADDR_IN);
	while (m_bAccepterRun) {
		// Get index
		stClientInfo* pClientInfo = GetEmptyClientInfo();
		if (pClientInfo == NULL) {
			printf("[Error] Location : CIOCP::AccepterThread, Reason : Num of client is full \n");
			return;
		}
		// Wait for connection
		pClientInfo->m_SocketClient = accept(m_ListenSocket, (SOCKADDR*)&stClientAddr, &nAddrLen);
		if (pClientInfo->m_SocketClient == INVALID_SOCKET) continue;
		// Store Address Information
		memcpy(&pClientInfo->m_saClientAddr, &stClientAddr, sizeof(SOCKADDR_IN));
		// Connect I/O Completion Port and socket 
		bool bRet = BindIOCompletionPort(pClientInfo);
		if (bRet == false) return;
		// Request Recv Overlapped I/O 
		bRet = BindRecv(pClientInfo);
		if (bRet == false) return;
		printf("A cllient has been connected: IP(%s) SOCKET(%d)", inet_ntoa(stClientAddr.sin_addr), pClientInfo->m_SocketClient);
		m_nClientCnt++;
	}
}

void CIOCP::WorkerThread()
{
	// Pointer to recv CompletionKey
	stClientInfo* pClientInfo = NULL;
	BOOL bSuccess = TRUE;
	// Sent size in Overlapped I/O 
	DWORD dwIoSize = 0;
	// Pointer to recv I/O  Overlapped Structure
	LPOVERLAPPED lpOverlapped = NULL;

	while (m_bWorkerRun) {
		cout << "Worker thread is running!" << endl;
		bSuccess = GetQueuedCompletionStatus(m_hIOCP,
			&dwIoSize, // Sent byte
			(LPDWORD)&pClientInfo, // CompletionKey
			&lpOverlapped, // Overlapped IO Object
			INFINITE); // Waiting Time

		if (bSuccess && 64 != GetLastError() == FALSE) {
			printf("[Error] Location : CIOCP::WorkerThread, Reason : GetQueuedCompletionStatus() has been failed: %d \n", WSAGetLastError());
		}

		// A client is disconnected
		if (bSuccess == FALSE && dwIoSize == 0) {
			printf("SOCKET(%d) is disconnected \n", pClientInfo->m_SocketClient);
			if (pClientInfo->m_eLocation == eGAME_ROOM) {
				pClientInfo->m_eLocation = eLOGOUT;
				int type = eLOGOUT;
			}
			CloseSocket(pClientInfo);
			continue;
		}

		// A thread is closed
		if (bSuccess == TRUE && dwIoSize == 0 && lpOverlapped == NULL) {
			m_bWorkerRun = false;
			continue;
		}

		if (NULL == lpOverlapped)continue;
		stOverlappedEx* pOverlappedEx = (stOverlappedEx*)lpOverlapped;
		// Overlapped I/O Recv
		if (pOverlappedEx->m_eOperation == eOP_RECV) {
			pOverlappedEx->m_szBuf[dwIoSize] = NULL;
			printf("[Recv] bytes: %d IP(%s) SOCKET(%d)", dwIoSize, inet_ntoa(pClientInfo->m_saClientAddr.sin_addr), pClientInfo->m_SocketClient);

			//m_bRetval = DivideUserInfo(pOverlappedEx->m_szBuf, pClientInfo);


					//SendMsg(pClientInfo, buf, MAX_MSGSIZE + sizeof(int));

		}

		else if (pOverlappedEx->m_eOperation == eOP_SEND) {
			printf("[Send] bytes: %d IP(%s) SOCKET(%d)", dwIoSize, inet_ntoa(pClientInfo->m_saClientAddr.sin_addr), pClientInfo->m_SocketClient);
		}
			
		// Exception
		else {
			printf("socket(%d) Exception", pClientInfo->m_SocketClient);
		}

	}
}

void CIOCP::DestroyThread()
{
	for (int i = 0; i < MAX_WORKER_THREAD; ++i) {
		// Send close message to thread whitch is waiting in WaitingThread Queue
		PostQueuedCompletionStatus(m_hIOCP, 0, 0, NULL);
	}
	for (int i = 0; i < MAX_WORKER_THREAD; ++i) {
		// Close Handle and Wait till thread is closed
		CloseHandle(m_hWorkerThread[i]);
		WaitForSingleObject(m_hWorkerThread[i], INFINITE);
	}
	m_bAccepterRun = false;
	// Close listen socket 
	closesocket(m_ListenSocket);
	// Wait for thread
	WaitForSingleObject(m_hAccepterThread, INFINITE);

}