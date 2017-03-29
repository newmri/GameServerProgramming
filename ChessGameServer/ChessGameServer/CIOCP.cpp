#include "CIOCP.h"
#include <process.h>

CIOCP* CIOCP::m_pInstance = NULL;

// to proccess WSARecv and WSASend 
unsigned int  WINAPI CallWorkerThread(LPVOID a_p)
{
	CIOCP* pcOverlappedEvent = (CIOCP*)a_p;
	pcOverlappedEvent->WorkerThread();
	return 0;
}

// SingleTon

CIOCP* CIOCP::Instance()
{
	if (!m_pInstance) m_pInstance = new CIOCP;
	return m_pInstance;
}

void CIOCP::DestroyInstance()
{
	if (m_pInstance) {
		delete m_pInstance;
		m_pInstance = NULL;
	}
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

	printf("Initializing socket has been succeeded \n");
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
	printf("Enrollment Success! \n");
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
	printf("Worker thread is starting! \n");
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
	printf("Server is started!");
	bool bRet = CreateWorkerThread();
	if (bRet == false) return;

	SOCKADDR_IN stClientAddr;
	int nAddrLen = sizeof(SOCKADDR_IN);
	while (true) {
		// Get index
		stClientInfo* stpClientInfo = GetEmptyClientInfo();
		if (stpClientInfo == NULL) {
			printf("[Error] Location : CIOCP::AccepterThread, Reason : Num of client is full \n");
			return;
		}
		// Wait for connection
		stpClientInfo->m_SocketClient = accept(m_ListenSocket, (SOCKADDR*)&stClientAddr, &nAddrLen);
		if (stpClientInfo->m_SocketClient == INVALID_SOCKET) continue;
		// Store Address Information
		memcpy(&stpClientInfo->m_saClientAddr, &stClientAddr, sizeof(SOCKADDR_IN));
		// Connect I/O Completion Port and socket 
		bool bRet = BindIOCompletionPort(stpClientInfo);
		if (bRet == false) return;

		printf("A cllient has been connected: IP(%s) SOCKET(%d) \n", inet_ntoa(stClientAddr.sin_addr), stpClientInfo->m_SocketClient);
		
		stpClientInfo->m_eLocation = eGAME_ROOM;

		enumDataType eDataType = eMOVE;
		SetNewClientInfo(stpClientInfo);
		// To new client
		AssembleAndSendPacket(stpClientInfo, eDataType);
		m_nClientCnt++;

		SetFirstChessPos();

		SearchOldClientInfo(stpClientInfo);
	}

}

void CIOCP::SetNewClientInfo(stClientInfo* a_stpClientInfo)
{
	a_stpClientInfo->m_usId = m_nClientCnt + 1;
	a_stpClientInfo->m_pos.x = m_pos.x;
	a_stpClientInfo->m_pos.y = m_pos.y;
}

void CIOCP::SetFirstChessPos()
{
	if (m_pos.x != eRIGHT_END - MOVE_PIXEL) m_pos.x += MOVE_PIXEL;
	else {
		m_pos.x = CHESS_FIRST_X;
		m_pos.y += MOVE_PIXEL;
	}
}

void CIOCP::SearchOldClientInfo(stClientInfo* a_stpNewClientInfo)
{
	for (int i = 0; i < m_nClientCnt; ++i) {
		enumDataType eDataType = eCLIENT_INFO;
		if (m_stpClientInfo[i].m_eLocation == eGAME_ROOM && m_stpClientInfo[i].m_usId != a_stpNewClientInfo->m_usId) {
			m_stpClientInfo[i].m_eAnotherLocation = a_stpNewClientInfo->m_eLocation;
			m_stpClientInfo[i].m_usAnotherId = a_stpNewClientInfo->m_usId;
			m_stpClientInfo[i].m_AnotherPos = a_stpNewClientInfo->m_pos;
			// To Old client
			AssembleAndSendPacket(&m_stpClientInfo[i], eDataType);
			// To new client
			a_stpNewClientInfo->m_eAnotherLocation = m_stpClientInfo[i].m_eLocation;
			a_stpNewClientInfo->m_usAnotherId = m_stpClientInfo[i].m_usId;
			a_stpNewClientInfo->m_AnotherPos = m_stpClientInfo[i].m_pos;
			AssembleAndSendPacket(a_stpNewClientInfo, eDataType);
		}
	}
}

void CIOCP::AssembleAndSendPacket(stClientInfo* a_stpClientInfo, const enumDataType& a_eDataType)
{
	switch (a_eDataType) {
	case eMOVE: {
		int nCommand = eMOVE;

		stSimpleClientInfo stSimpleClientInfo;
		stSimpleClientInfo.m_eLocation = a_stpClientInfo->m_eLocation;
		stSimpleClientInfo.m_usId = a_stpClientInfo->m_usId;
		stSimpleClientInfo.m_pos = a_stpClientInfo->m_pos;

		m_nBufLen = sizeof(nCommand) + sizeof(stSimpleClientInfo);
		// len
		memcpy(a_stpClientInfo->m_stSendOverlappedEx.m_szBuf, &m_nBufLen, sizeof(m_nBufLen));
		// command
		memcpy(&a_stpClientInfo->m_stSendOverlappedEx.m_szBuf[sizeof(m_nBufLen)], &nCommand, sizeof(int));
		// client info
		memcpy(&a_stpClientInfo->m_stSendOverlappedEx.m_szBuf[sizeof(m_nBufLen) + sizeof(nCommand)], (char*)&stSimpleClientInfo, sizeof(stSimpleClientInfo));
		
		m_nBufLen += sizeof(int);
		SendMsg(a_stpClientInfo, a_stpClientInfo->m_stSendOverlappedEx.m_szBuf, m_nBufLen);
		break;
	}
	case eCLIENT_INFO: {
		int nCommand = eCLIENT_INFO;

		stSimpleClientInfo stSimpleClientInfo;
		stSimpleClientInfo.m_eLocation = a_stpClientInfo->m_eAnotherLocation;
		stSimpleClientInfo.m_usId = a_stpClientInfo->m_usAnotherId;
		stSimpleClientInfo.m_pos = a_stpClientInfo->m_AnotherPos;

		m_nBufLen = sizeof(nCommand) + sizeof(stSimpleClientInfo);
		// len
		memcpy(&a_stpClientInfo->m_stSendOverlappedEx.m_szBuf, &m_nBufLen, sizeof(m_nBufLen));
		// command
		memcpy(&a_stpClientInfo->m_stSendOverlappedEx.m_szBuf[sizeof(m_nBufLen)], &nCommand, sizeof(nCommand));
		// client info
		memcpy(&a_stpClientInfo->m_stSendOverlappedEx.m_szBuf[sizeof(m_nBufLen) + sizeof(nCommand)], (char*)&stSimpleClientInfo, sizeof(stSimpleClientInfo));

		m_nBufLen += sizeof(int);
		SendMsg(a_stpClientInfo, a_stpClientInfo->m_stSendOverlappedEx.m_szBuf, m_nBufLen);
		break;
	}
	case eANOTHER_MOVE: {
		int nCommand = eANOTHER_MOVE;
		stSimpleClientInfo stSimpleClientInfo;
		stSimpleClientInfo.m_eLocation = a_stpClientInfo->m_eAnotherLocation;
		stSimpleClientInfo.m_usId = a_stpClientInfo->m_usAnotherId;
		stSimpleClientInfo.m_pos = a_stpClientInfo->m_AnotherPos;

		m_nBufLen = sizeof(nCommand) + sizeof(stSimpleClientInfo);
		// len
		memcpy(a_stpClientInfo->m_stSendOverlappedEx.m_szBuf, &m_nBufLen, sizeof(m_nBufLen));
		// command
		memcpy(&a_stpClientInfo->m_stSendOverlappedEx.m_szBuf[sizeof(m_nBufLen)], &nCommand, sizeof(int));
		// client info
		memcpy(&a_stpClientInfo->m_stSendOverlappedEx.m_szBuf[sizeof(m_nBufLen) + sizeof(nCommand)], (char*)&stSimpleClientInfo, sizeof(stSimpleClientInfo));

		m_nBufLen += sizeof(int);
		SendMsg(a_stpClientInfo, a_stpClientInfo->m_stSendOverlappedEx.m_szBuf, m_nBufLen);
		break;
	}
	default:
		break;

	}
}

void CIOCP::DisassemblePacket(stClientInfo* a_stpClientInfo)
{
	switch (a_stpClientInfo->m_stRecvOverlappedEx.m_szBuf[sizeof(int)]) {
	case eMOVE: {
		stSimpleClientInfo stSimpleClientInfo;
		memcpy(&stSimpleClientInfo, &a_stpClientInfo->m_stRecvOverlappedEx.m_szBuf[sizeof(int) * 2], sizeof(stSimpleClientInfo));

		if (stSimpleClientInfo.m_pos.x == eLEFT_END ||stSimpleClientInfo.m_pos.x == eRIGHT_END) {
		enumDataType eDataType = eMOVE;
		AssembleAndSendPacket(a_stpClientInfo, eDataType);
		return;
		}
		if (stSimpleClientInfo.m_pos.y == eTOP_END  || stSimpleClientInfo.m_pos.y == eBOTTOM_END) {
		enumDataType eDataType = eMOVE;
		AssembleAndSendPacket(a_stpClientInfo, eDataType);
		return;
		}

		a_stpClientInfo->m_pos = stSimpleClientInfo.m_pos;
		enumDataType eDataType = eMOVE;
		
		// To the moved client
		AssembleAndSendPacket(a_stpClientInfo, eDataType);

		if(m_nClientCnt != 1) eDataType = eANOTHER_MOVE;
		for (int i = 0; i < m_nClientCnt; ++i) {
			if (m_stpClientInfo[i].m_eLocation == eGAME_ROOM && m_stpClientInfo[i].m_usId != stSimpleClientInfo.m_usId) {
				m_stpClientInfo[i].m_eAnotherLocation = a_stpClientInfo->m_eLocation;
				m_stpClientInfo[i].m_AnotherPos = a_stpClientInfo->m_pos;
				m_stpClientInfo[i].m_usAnotherId = a_stpClientInfo->m_usId;
				AssembleAndSendPacket(&m_stpClientInfo[i], eDataType);
			}
		}
		break;
	}
	}
	}

bool CIOCP::BindRecv(stClientInfo* a_pClientInfo)
{
	DWORD dwFlag = 0;
	DWORD dwRecvNumBytes = 0;

	// Set Overlapped I/O Information
	a_pClientInfo->m_stRecvOverlappedEx.m_wsaBuf.len = MAX_BUF_SIZE;
	a_pClientInfo->m_stRecvOverlappedEx.m_wsaBuf.buf = a_pClientInfo->m_stRecvOverlappedEx.m_szBuf;
	a_pClientInfo->m_stRecvOverlappedEx.m_eOperation = eOP_RECV;

	int nRet = WSARecv(a_pClientInfo->m_SocketClient, &(a_pClientInfo->m_stRecvOverlappedEx.m_wsaBuf), 1, &dwRecvNumBytes, &dwFlag, (LPWSAOVERLAPPED)&(a_pClientInfo->m_stRecvOverlappedEx), NULL);

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
		printf("A cllient has been connected: IP(%s) SOCKET(%d) \n", inet_ntoa(stClientAddr.sin_addr), pClientInfo->m_SocketClient);
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
		printf("Worker thread is running! \n");
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

		if (lpOverlapped == NULL) continue;
		stOverlappedEx* stpOverlappedEx = (stOverlappedEx*)lpOverlapped;
		// Overlapped I/O Recv
		if (stpOverlappedEx->m_eOperation == eOP_RECV) {
			stpOverlappedEx->m_szBuf[dwIoSize] = NULL;
			printf("[Recv] bytes: %d IP(%s) SOCKET(%d) \n", dwIoSize, inet_ntoa(pClientInfo->m_saClientAddr.sin_addr), pClientInfo->m_SocketClient);
		
			DisassemblePacket(pClientInfo);

		}

		else if (stpOverlappedEx->m_eOperation == eOP_SEND) {
			printf("[Send] bytes: %d IP(%s) SOCKET(%d) \n", dwIoSize, inet_ntoa(pClientInfo->m_saClientAddr.sin_addr), pClientInfo->m_SocketClient);
			
			// Request Recv Overlapped I/O 
			bool bRet = BindRecv(pClientInfo);
			if (bRet == false) continue;
		}
			
		// Exception
		else {
			printf("socket(%d) Exception", pClientInfo->m_SocketClient);
			continue;
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