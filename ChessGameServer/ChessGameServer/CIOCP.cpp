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
		cout<<"[Error] WSAStartup() has been failed: " << WSAGetLastError() << endl;
		return false;
	}
		
	// Construct TCP Overlapped I/O Socket
	m_ListenSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, NULL, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == m_ListenSocket) {
		cout << "[Error] WSASocket() has been failed: " << WSAGetLastError() << endl;
		return false;
	}

	cout << "Initializing socket has been succeeded" << endl;
	return true;
}


void CIOCP::CloseSocket(stClientInfo* a_a_pClientInfo, bool a_bIsForce)
{
	struct linger stLinger = { 0,0 }; // SO_DONTLINGER

	// Is closed by force?									  
	if (true == a_bIsForce)
		stLinger.l_onoff = 1;

	// Stop send and recv 
	shutdown(a_a_pClientInfo->m_SocketClient, SD_BOTH);
	// Set the option
	setsockopt(a_a_pClientInfo->m_SocketClient, SOL_SOCKET, SO_LINGER, (char*)&stLinger, sizeof(stLinger));
	// Close Connection
	closesocket(a_a_pClientInfo->m_SocketClient);
	a_a_pClientInfo->m_SocketClient = INVALID_SOCKET;
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

bool CIOCP::StartServer()
{

	bool bRet = CreateWorkerThread();
	if (false == bRet)return false;
	bRet = CreateAccepterThread();
	if (false == bRet)return false;

	// Create CompletionPort Object
	m_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
	if (NULL == m_hIOCP) {
		printf("[Error] Location : CIOCP::StartServer, Reason : CreateIoCompletionPort() has been failed: %d \n", WSAGetLastError());
		return false;
	}
	cout << "Server is started!" << endl;
	return false;
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