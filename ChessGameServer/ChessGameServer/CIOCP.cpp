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

// to proccess Accept 
unsigned int  WINAPI CallAccepterThread(LPVOID a_p)
{
	CIOCP* pcOverlappedEvent = (CIOCP*)a_p;
	pcOverlappedEvent->AccepterThread();
	return 0;
}

unsigned int  WINAPI CallTimerThread(LPVOID a_p)
{
	CIOCP* pcOverlappedEvent = (CIOCP*)a_p;
	pcOverlappedEvent->TimerThread();
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
	ZeroMemory(m_szSocketBuf, MAX_BUF_SIZE);
	for (int i = 0; i < MAX_WORKER_THREAD; i++)
		m_hWorkerThread[i] = NULL;

	// Contruct structures that have client's information 
	m_stpClientInfo = new stClientInfo[MAX_CLIENT_NUM];
	m_pos.x = CHESS_FIRST_X;
	m_pos.y = CHESS_FIRST_Y;
	m_wId = 0;


	for (WORD i = 0; i < MAX_NPC_NUM; ++i) {

		m_cNPC[i].Init();
	}

	srand((unsigned int)time(NULL));
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


void CIOCP::CloseSocket(const WORD& a_wId, const bool& a_bIsForce)
{
	m_stpClientInfo[a_wId].m_bIsConnected = false;
	m_stpClientInfo[a_wId].m_bIsLogined = false;
	DBInfo Info;
	strcpy(Info.ID, m_stpClientInfo[a_wId].m_ID);
	Info.Pos_X = m_stpClientInfo[a_wId].m_pos.x;
	Info.Pos_Y = m_stpClientInfo[a_wId].m_pos.y;
	m_CDB.Update(Info);

	m_nClientCnt--;
	
	struct linger stLinger = { 0,0 }; // SO_DONTLINGER

	// Is closed by force?									  
	if (true == a_bIsForce)
		stLinger.l_onoff = 1;

	// Stop send and recv 
	shutdown(m_stpClientInfo[a_wId].m_SocketClient, SD_BOTH);
	// Set the option
	setsockopt(m_stpClientInfo[a_wId].m_SocketClient, SOL_SOCKET, SO_LINGER, (char*)&stLinger, sizeof(stLinger));
	// Close Connection
	closesocket(m_stpClientInfo[a_wId].m_SocketClient);

	m_stpClientInfo[a_wId].m_SocketClient = INVALID_SOCKET;

	unordered_set<WORD> lvl;
	m_stpClientInfo[a_wId].m_lock.lock();
	lvl = m_stpClientInfo[a_wId].m_view_list;
	m_stpClientInfo[a_wId].m_lock.unlock();

	// Notify to everyone who is near
	for (auto target : lvl){
		m_stpClientInfo[target].m_lock.lock();
		if (m_stpClientInfo[target].m_view_list.count(a_wId) != 0){
			m_stpClientInfo[target].m_view_list.erase(a_wId);
			m_stpClientInfo[target].m_lock.unlock();
			SendRemoveClient(target, a_wId);
		}
		else m_stpClientInfo[target].m_lock.unlock();	
	}
	m_stpClientInfo[a_wId].m_lock.lock();
	m_stpClientInfo[a_wId].m_view_list.clear();
	m_stpClientInfo[a_wId].m_lock.unlock();

	m_stpClientInfo[a_wId].m_NPC_Lock.lock();
	m_stpClientInfo[a_wId].m_NPC_view_list.clear();
	m_stpClientInfo[a_wId].m_NPC_Lock.unlock();



	printf("SOCKET(%d) is disconnected \n", m_stpClientInfo[a_wId].m_SocketClient);

}


const bool& CIOCP::CreateWorkerThread()
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

const bool& CIOCP::CreateAccepterThread()
{
	unsigned int uiThreadId = 0;

	// Thread Creation to input into WaitingThread Queue

	m_hAccepterThread = (HANDLE)_beginthreadex(NULL, 0, &CallAccepterThread, this, CREATE_SUSPENDED, &uiThreadId);
	if (m_hAccepterThread == NULL) {
		printf("[Error] Location : CIOCP::CreateAccepterThread, Reason : CallAccepterThread() has been failed: %d \n", WSAGetLastError());
		return false;
	}
	ResumeThread(m_hAccepterThread);
	printf("Accepter thread is starting! \n");
	return true;
}

const bool&  CIOCP::CreateTimerThread()
{
	unsigned int uiThreadId = 0;

	// Thread Creation to input into WaitingThread Queue

	m_hTimerThread = (HANDLE)_beginthreadex(NULL, 0, &CallTimerThread, this, CREATE_SUSPENDED, &uiThreadId);
	if (m_hTimerThread == NULL) {
		printf("[Error] Location : CIOCP::CreateTimerThread, Reason : CallTimerThread() has been failed: %d \n", WSAGetLastError());
		return false;
	}
	ResumeThread(m_hTimerThread);
	printf("Timer thread is starting! \n");
	return true;
}

const bool& CIOCP::BindAndRecvIOCompletionPort(const WORD& a_wNewId)
{
	HANDLE hIOCP;
	DWORD DwRecvFlag = 0;
	// Connect socket and pClientInfo into COmpletionPort object
	hIOCP = CreateIoCompletionPort(reinterpret_cast<HANDLE>(m_stpClientInfo[a_wNewId].m_SocketClient), m_hIOCP, a_wNewId, 0);
	if (hIOCP == NULL || m_hIOCP != hIOCP) {
		printf("[Error] Location : CIOCP::BindIOCompletionPort, Reason : CreateIoCompletionPort() has been failed: %d \n", WSAGetLastError());
		return false;
	}
	WSARecv(m_stpClientInfo[a_wNewId].m_SocketClient, &m_stpClientInfo[a_wNewId].m_stRecvOverlappedEx.m_wsaBuf, 1,
		NULL, &DwRecvFlag, &(m_stpClientInfo[a_wNewId].m_stRecvOverlappedEx.m_wsaOverlapped), NULL);

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
	printf("Server is started! \n");
	bool bRet = CreateWorkerThread();
	if (bRet == false) return;

	bRet = CreateAccepterThread();
	if (bRet == false) return;

	bRet = CreateTimerThread();
	if (bRet == false) return;

	while (m_bAccepterRun);
}

const bool& CIOCP::BindandListen(const int& a_nBindPort)
{
	SOCKADDR_IN stServerAddr;
	stServerAddr.sin_family = AF_INET;
	stServerAddr.sin_port = htons(a_nBindPort);
	stServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	// bind
	int nRet = ::bind(m_ListenSocket, (SOCKADDR*)&stServerAddr, sizeof(SOCKADDR_IN));
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

void CIOCP::SetNewClientInfo(const WORD& a_wNewId)
{
	m_stpClientInfo[a_wNewId].m_bIsConnected = true;
	m_stpClientInfo[a_wNewId].m_pos.x = CHESS_FIRST_X;
	m_stpClientInfo[a_wNewId].m_pos.y = CHESS_FIRST_Y;
	m_stpClientInfo[a_wNewId].wCurrPacketSize = 0;
	m_stpClientInfo[a_wNewId].wPrevPacketData = 0;
	ZeroMemory(&m_stpClientInfo[a_wNewId].m_stRecvOverlappedEx, sizeof(m_stpClientInfo[a_wNewId].m_stRecvOverlappedEx));
	m_stpClientInfo[a_wNewId].m_stRecvOverlappedEx.m_eOperation = eOP_RECV;

	m_stpClientInfo[a_wNewId].m_stRecvOverlappedEx.m_wsaBuf.buf =
		reinterpret_cast<CHAR *>(m_stpClientInfo[a_wNewId].m_stRecvOverlappedEx.m_szBuf);
	m_stpClientInfo[a_wNewId].m_stRecvOverlappedEx.m_wsaBuf.len = sizeof(m_stpClientInfo[a_wNewId].m_stRecvOverlappedEx.m_szBuf);
}

const bool& CIOCP::IsClose(const WORD& a_wFrom, const WORD& a_wTo)
{
	return sqrt(((m_stpClientInfo[a_wFrom].m_pos.x - m_stpClientInfo[a_wTo].m_pos.x) *
		(m_stpClientInfo[a_wFrom].m_pos.x - m_stpClientInfo[a_wTo].m_pos.x)) +
		((m_stpClientInfo[a_wFrom].m_pos.y - m_stpClientInfo[a_wTo].m_pos.y) *
		(m_stpClientInfo[a_wFrom].m_pos.y - m_stpClientInfo[a_wTo].m_pos.y))) < MAX_VIEW;
}

const bool& CIOCP::IsCloseWithNPC(const WORD& a_wFrom, const WORD& a_wTo)
{
	return sqrt(((m_cNPC[a_wFrom].GetPos().x - m_stpClientInfo[a_wTo].m_pos.x) *
		(m_cNPC[a_wFrom].GetPos().x - m_stpClientInfo[a_wTo].m_pos.x)) +
		((m_cNPC[a_wFrom].GetPos().y - m_stpClientInfo[a_wTo].m_pos.y) *
		(m_cNPC[a_wFrom].GetPos().y - m_stpClientInfo[a_wTo].m_pos.y))) < MAX_VIEW;
}
void CIOCP::AccepterThread()
{
	SOCKADDR_IN stClientAddr;
	ZeroMemory(&stClientAddr, sizeof(SOCKADDR_IN));
	stClientAddr.sin_family = AF_INET;
	stClientAddr.sin_port = htons(SERVER_PORT);
	stClientAddr.sin_addr.s_addr = INADDR_ANY;
	int nAddrLen = sizeof(stClientAddr);

	while (m_bAccepterRun) {

		WORD wNewId = 65535;

		for (int i = 0; i < MAX_CLIENT_NUM; ++i) if (m_stpClientInfo[i].m_bIsConnected == false) { wNewId = i; break; }
		
		if (wNewId == 65535) { std::cout << "MAX USER OVERFLOW!\n"; CloseSocket(wNewId);  continue; }

		// Wait for connection
		m_stpClientInfo[wNewId].m_SocketClient = WSAAccept(m_ListenSocket, reinterpret_cast<sockaddr*>(&stClientAddr), &nAddrLen, NULL, NULL);
		if (m_stpClientInfo[wNewId].m_SocketClient == INVALID_SOCKET) {
			int nErrNo = WSAGetLastError();
			DisPlayError("WSAAccept:", nErrNo);
			continue;
		}
		
		SetNewClientInfo(wNewId);

		DWORD recv_flag = 0;
		// Connect I/O Completion Port and socket 
		bool bRet = BindAndRecvIOCompletionPort(wNewId);
		if (bRet == false) return;
		else {
			printf("A cllient has been connected: IP(%s) SOCKET(%d) \n", inet_ntoa(stClientAddr.sin_addr), m_stpClientInfo[wNewId].m_SocketClient);
			m_nClientCnt++;
		}

		
	}
}

void CIOCP::SendLoginFail(const WORD& a_wClient, const LOGIN a_login)
{
	ST_SC_LOGIN_RESULT stPacket;
	stPacket.m_bytSize = sizeof(stPacket);
	stPacket.m_bytType = a_login;
	
	SendPacket(a_wClient, &stPacket);

}
void CIOCP::SendLoginSuccess(const WORD& a_wClient)
{
	ST_SC_LOGIN_RESULT stPacket;
	stPacket.m_bytSize = sizeof(stPacket);
	stPacket.m_bytType = eSC_LOGIN_SUCCESS;
	strcpy(stPacket.m_ID, m_stpClientInfo[a_wClient].m_ID);
	SendPacket(a_wClient, &stPacket);
}
void CIOCP::LoginSuccessProcess(const WORD& wNewId)
{
	m_stpClientInfo[wNewId].m_bIsLogined = true;

	SendLoginSuccess(wNewId);

	SendPutClient(wNewId, wNewId);

	unordered_set<WORD> local_view_list;
	unordered_set<WORD> local_NPC_view_list;

	// Client view
	for (int i = 0; i < MAX_CLIENT_NUM; ++i){
		if (m_stpClientInfo[i].m_bIsLogined)
			if (i != wNewId) {
				if (IsClose(i, wNewId)){
					SendPutClient(wNewId, i);
					local_view_list.insert(i);
					SendPutClient(i, wNewId);
					m_stpClientInfo[wNewId].m_lock.lock();
					m_stpClientInfo[i].m_view_list.insert(wNewId);
					m_stpClientInfo[wNewId].m_lock.unlock();
				}
			}
	}


	m_stpClientInfo[wNewId].m_lock.lock();
	for (auto p : local_view_list) m_stpClientInfo[wNewId].m_view_list.insert(p);
	m_stpClientInfo[wNewId].m_lock.unlock();

	// NPC view
	for (int i = 0; i < MAX_NPC_NUM; ++i) {
		if (IsCloseWithNPC(i, wNewId)) {
			SendPutNPC(wNewId, i);
			local_NPC_view_list.insert(i);
		}
	}

	m_stpClientInfo[wNewId].m_NPC_Lock.lock();
	for (auto p : local_NPC_view_list) m_stpClientInfo[wNewId].m_NPC_view_list.insert(p);
	m_stpClientInfo[wNewId].m_NPC_Lock.unlock();
	

}
void CIOCP::SendPutClient(const WORD& a_wClient, const WORD& a_wObject)
{
	ST_SC_PUT_OBJECT stPacket;
	stPacket.m_wId = a_wObject;
	stPacket.m_bytSize = sizeof(stPacket);
	stPacket.m_bytType = eSC_PUT_CLIENT;
	stPacket.m_wX = m_stpClientInfo[a_wObject].m_pos.x;
	stPacket.m_wY = m_stpClientInfo[a_wObject].m_pos.y;

	SendPacket(a_wClient, &stPacket);
}


void CIOCP::SendMoveClient(const WORD& a_wClient, const WORD& a_wObject)
{
	ST_SC_MOVE_OBJECT stPacket;
	stPacket.m_wId = a_wObject;
	stPacket.m_bytSize = sizeof(stPacket);
	stPacket.m_bytType = eSC_MOVE_CLIENT;
	stPacket.m_wX = m_stpClientInfo[a_wObject].m_pos.x;
	stPacket.m_wY = m_stpClientInfo[a_wObject].m_pos.y;

	SendPacket(a_wClient, &stPacket);
}

void CIOCP::SendRemoveClient(const WORD& a_wClient, const WORD& a_wObject)
{
	ST_SC_REMOVE_OBJECT stPacket;
	stPacket.m_wId = a_wObject;
	stPacket.m_bytSize = sizeof(stPacket);
	stPacket.m_bytType = eSC_REMOVE_CLIENT;

	SendPacket(a_wClient, &stPacket);

}

void CIOCP::SendPutNPC(const WORD& a_wClient, const WORD& a_wObject)
{
	ST_SC_PUT_OBJECT stPacket;
	stPacket.m_wId = a_wObject;
	stPacket.m_bytSize = sizeof(stPacket);
	stPacket.m_bytType = eSC_PUT_NPC;
	stPacket.m_wX = m_cNPC[a_wObject].GetPos().x;
	stPacket.m_wY = m_cNPC[a_wObject].GetPos().y;

	SendPacket(a_wClient, &stPacket);
}

void CIOCP::SendMoveNPC(const WORD& a_wClient, const WORD& a_wObject)
{
	ST_SC_MOVE_OBJECT stPacket;
	stPacket.m_wId = a_wObject;
	stPacket.m_bytSize = sizeof(stPacket);
	stPacket.m_bytType = eSC_MOVE_NPC;
	stPacket.m_wX = m_cNPC[a_wObject].GetPos().x;
	stPacket.m_wY = m_cNPC[a_wObject].GetPos().y;

	SendPacket(a_wClient, &stPacket);
}

void CIOCP::SendRemoveNPC(const WORD& a_wClient, const WORD& a_wObject)
{
	ST_SC_REMOVE_OBJECT stPacket;
	stPacket.m_wId = a_wObject;
	stPacket.m_bytSize = sizeof(stPacket);
	stPacket.m_bytType = eSC_REMOVE_NPC;

	SendPacket(a_wClient, &stPacket);

}

void CIOCP::DisPlayError(const char* a_msg, const int& nErr_no)
{
	WCHAR *lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, nErr_no,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	std::cout << a_msg;
	std::wcout << L"Error" << lpMsgBuf << std::endl;
	LocalFree(lpMsgBuf);
	while (true);
}

void CIOCP::HandleView(const WORD& a_wId)
{
	unordered_set<WORD> new_view_list;

	for (int i = 0; i < MAX_CLIENT_NUM; ++i) if (m_stpClientInfo[i].m_bIsLogined)
		if (i != a_wId) if (IsClose(a_wId, i)) new_view_list.insert(i);
		
	// Object to be added
	unordered_set<WORD> vlc;
	m_stpClientInfo[a_wId].m_lock.lock();
	vlc = m_stpClientInfo[a_wId].m_view_list;
	m_stpClientInfo[a_wId].m_lock.unlock();

	// Roop in my view-list
	for (auto target : new_view_list){
		// If I don't have target
		if (vlc.count(target) == 0){
			SendPutClient(a_wId, target);
			vlc.insert(target);

			m_stpClientInfo[target].m_lock.lock();
			// If Target have me
			if (m_stpClientInfo[target].m_view_list.count(a_wId) != 0){
				SendMoveClient(target, a_wId); // To target from me
				m_stpClientInfo[target].m_lock.unlock();
			}
			else{ // Target doesn't have me

				// From target to me
				m_stpClientInfo[target].m_view_list.insert(a_wId);
				m_stpClientInfo[target].m_lock.unlock();
				SendPutClient(target, a_wId);
			}

		}
		else{ // I have target
			m_stpClientInfo[target].m_lock.lock();
			if (m_stpClientInfo[target].m_view_list.count(a_wId) != 0){ // Target has me
				SendMoveClient(target, a_wId);
				m_stpClientInfo[target].m_lock.unlock();
			}
			else{ // I Don't have target
				m_stpClientInfo[target].m_view_list.insert(a_wId);
				m_stpClientInfo[target].m_lock.unlock();
				SendPutClient(target, a_wId);
			}
		}
	}


	// Now object is far away...
	unordered_set<WORD> removed_id_list;

	for (auto target : vlc){
		if (new_view_list.count(target) == 0){ // If target is far away
			SendRemoveClient(a_wId, target);
			removed_id_list.insert(target);

			m_stpClientInfo[target].m_lock.lock();
			if (m_stpClientInfo[target].m_view_list.count(a_wId) != 0){ // If target has me
				m_stpClientInfo[target].m_view_list.erase(a_wId);
				m_stpClientInfo[target].m_lock.unlock();
				SendRemoveClient(target, a_wId);
			}
		}
	}

	m_stpClientInfo[a_wId].m_lock.lock();
	for (auto p : vlc) m_stpClientInfo[a_wId].m_view_list.insert(p);
	for (auto d : removed_id_list) m_stpClientInfo[a_wId].m_view_list.erase(d);
	m_stpClientInfo[a_wId].m_lock.unlock();

	// HandleNPCView
	for (WORD i = 0; i < MAX_NPC_NUM; ++i) {
		if (IsCloseWithNPC(i, a_wId)) {
			WakeUpNPC(i);
			HandleNPCView(a_wId, i);
		}
	}
}

void CIOCP::HandleNPCView(const WORD& a_wId, const WORD& a_NPC)
{
	unordered_set<WORD> new_view_list;

	// Object to be added
	unordered_set<WORD> vlc;
	// Object to be removed
	unordered_set<WORD> removed_id_list;

	if (IsCloseWithNPC(a_NPC, a_wId)) new_view_list.insert(a_NPC);

	m_stpClientInfo[a_wId].m_NPC_Lock.lock();
	vlc = m_stpClientInfo[a_wId].m_NPC_view_list;
	m_stpClientInfo[a_wId].m_NPC_Lock.unlock();

	for (auto target : new_view_list) {
		// If Client doesn't have target NPC
		if (vlc.count(target) == 0) {
			SendPutNPC(a_wId, a_NPC);
			vlc.insert(target);
		}

		else {  // Client has target NPC
			SendMoveNPC(a_wId, a_NPC);
		}
	}

			// Now object is far away...
	for (auto target : vlc) {
		if (new_view_list.count(target) == 0) { // If target is far away
			SendRemoveNPC(a_wId, target);
			removed_id_list.insert(target);

		}
	}
	m_stpClientInfo[a_wId].m_NPC_Lock.lock();
	for (auto p : vlc) m_stpClientInfo[a_wId].m_NPC_view_list.insert(p);
	for (auto d : removed_id_list) m_stpClientInfo[a_wId].m_NPC_view_list.erase(d);
	m_stpClientInfo[a_wId].m_NPC_Lock.unlock();
		
}
void CIOCP::ProcessPacket(const WORD& a_wId, const unsigned char a_Packet[])
{
	switch (a_Packet[1]) {
	case eCS_UP: if (m_stpClientInfo[a_wId].m_pos.y > eTOP_END) m_stpClientInfo[a_wId].m_pos.y--; break;
	case eCS_DOWN: if (m_stpClientInfo[a_wId].m_pos.y < eBOTTOM_END) m_stpClientInfo[a_wId].m_pos.y++; break;
	case eCS_LEFT: if (m_stpClientInfo[a_wId].m_pos.x > eLEFT_END) m_stpClientInfo[a_wId].m_pos.x--; break;
	case eCS_RIGHT: if (m_stpClientInfo[a_wId].m_pos.x < eRIGHT_END) m_stpClientInfo[a_wId].m_pos.x++; break;
	case eCS_LOGIN: {
		char ID[ID_LEN];
		char PWD[PWD_LEN];
		memcpy(ID, (const void*)&a_Packet[4], a_Packet[2]);
		memcpy(PWD, (const void*)&a_Packet[4 + ID_LEN], a_Packet[3]);
		ID[a_Packet[2]] = '\0';
		PWD[a_Packet[3]] = '\0';

		for (int i = 0; i < MAX_CLIENT_NUM; ++i) {
			if (strcmp(ID, m_stpClientInfo[i].m_ID) == 0 && m_stpClientInfo[i].m_bIsLogined) {
				SendLoginFail(a_wId,LOGIN::eSC_LOGIN_FAIL_LOGINED);
				return;
			}
		}
		DBInfo pos = m_CDB.Login(ID, PWD);
		if (pos.Pos_X == -1) SendLoginFail(a_wId,LOGIN::eSC_LOGIN_FAIL_INCORRECT);
		else {
			strcpy(m_stpClientInfo[a_wId].m_ID, pos.ID);
			m_stpClientInfo[a_wId].m_pos.x = pos.Pos_X;  m_stpClientInfo[a_wId].m_pos.y = pos.Pos_Y;
			LoginSuccessProcess(a_wId);
		}
		return;
	}
	default: printf("Unknown Packet Type from Client : "); while (true);
	}

	SendMoveClient(a_wId, a_wId);

	HandleView(a_wId);
	
}


void CIOCP::SendPacket(const WORD& a_wId, void* a_vPacket)
{
	int nPsize = reinterpret_cast<unsigned char *>(a_vPacket)[0];
	int nPtype = reinterpret_cast<unsigned char *>(a_vPacket)[1];	
	stOverlappedEx* stOver = new stOverlappedEx;
	stOver->m_eOperation = eOP_SEND;
	memcpy(stOver->m_szBuf, a_vPacket, nPsize);
	ZeroMemory(&stOver->m_wsaOverlapped, sizeof(stOver->m_wsaOverlapped));
	stOver->m_wsaBuf.buf = reinterpret_cast<CHAR *>(stOver->m_szBuf);
	stOver->m_wsaBuf.len = nPsize;

	int nRet = WSASend(m_stpClientInfo[a_wId].m_SocketClient, &stOver->m_wsaBuf, 1, NULL, 0,
		&stOver->m_wsaOverlapped, NULL);

	if (0 != nRet) {
		int err_no = WSAGetLastError();
		if (WSA_IO_PENDING != err_no)
			DisPlayError("Error in SendPacket: \n", err_no);
	}


}

void CIOCP::WorkerThread()
{
	BOOL bSuccess = TRUE;
	// Sent size in Overlapped I/O 
	DWORD dwIoSize = 0;
	// Pointer to recv I/O  Overlapped Structure
	stOverlappedEx* stpOverlappedEx = nullptr;
	unsigned long long Id{};
	while (m_bWorkerRun) {
		//printf("Worker thread is running! \n");
		bSuccess = GetQueuedCompletionStatus(m_hIOCP,
			&dwIoSize, // Sent byte
			(LPDWORD)&Id, // CompletionKey
			reinterpret_cast<LPWSAOVERLAPPED*>(&stpOverlappedEx), // Overlapped IO Object
			INFINITE); // Waiting Time

		if (bSuccess == FALSE && GetLastError() != 64) {
			printf("[Error] Location : CIOCP::WorkerThread, Reason : GetQueuedCompletionStatus() has been failed: %d \n", WSAGetLastError());
		}

		// A client is disconnected
		if (bSuccess == FALSE) {
			int nErrNo = WSAGetLastError();
			if (nErrNo == 64) {
				CloseSocket(Id, true);
				continue;
			}
			else DisPlayError("GQCS : ", WSAGetLastError());
		}

		if(dwIoSize == 0){
			CloseSocket(Id);
			continue;
		}

		// A thread is closed
		if (bSuccess == TRUE && dwIoSize == 0 && stpOverlappedEx == nullptr) {
			m_bWorkerRun = false;
			continue;
		}

		if (stpOverlappedEx == nullptr) continue;

		// Overlapped I/O Recv
		if (stpOverlappedEx->m_eOperation == eOP_RECV) {
			//printf("[Recv] bytes: %d IP(%s) SOCKET(%d) \n", dwIoSize, inet_ntoa(m_stpClientInfo[Id].m_saClientAddr.sin_addr), m_stpClientInfo[Id].m_SocketClient);
			unsigned char *buf = m_stpClientInfo[Id].m_stRecvOverlappedEx.m_szBuf;
			unsigned psize = m_stpClientInfo[Id].wCurrPacketSize;
			unsigned pr_size = m_stpClientInfo[Id].wPrevPacketData;
			while (dwIoSize > 0) {
				if (psize == 0) psize = buf[0];
				if (dwIoSize + pr_size >= psize) {
					// Now can assemble packet
					unsigned char packet[MAX_PACKET_SIZE];
					memcpy(packet, m_stpClientInfo[Id].m_szPacketBuf, pr_size);
					memcpy(packet + pr_size, buf, psize - pr_size);
					ProcessPacket(static_cast<WORD>(Id), packet);
					dwIoSize -= psize - pr_size;
					buf += psize - pr_size;
					psize = 0; pr_size = 0;
				}
				else {
					memcpy(m_stpClientInfo[Id].m_szPacketBuf + pr_size, buf, dwIoSize);
					pr_size += dwIoSize;
					dwIoSize = 0;
				}
			}
			m_stpClientInfo[Id].wCurrPacketSize = psize;
			m_stpClientInfo[Id].wPrevPacketData = pr_size;
			DWORD recv_flag = 0;
			WSARecv(m_stpClientInfo[Id].m_SocketClient,
				&m_stpClientInfo[Id].m_stRecvOverlappedEx.m_wsaBuf, 1,
				NULL, &recv_flag, &m_stpClientInfo[Id].m_stRecvOverlappedEx.m_wsaOverlapped, NULL);

		}

		else if (stpOverlappedEx->m_eOperation == eOP_SEND) {
			if (dwIoSize != stpOverlappedEx->m_wsaBuf.len) {
				printf("Send Incomplete Error! \n");
				CloseSocket(Id,true);
			}

			//printf("[Send] bytes: %d IP(%s) SOCKET(%d) \n", dwIoSize, inet_ntoa(m_stpClientInfo[Id].m_saClientAddr.sin_addr), m_stpClientInfo[Id].m_SocketClient);
			delete stpOverlappedEx;

		}

		else if (stpOverlappedEx->m_eOperation == eOP_DO_AI) {
			m_cNPC[Id].Move();
			for (int i = 0; i < MAX_CLIENT_NUM; ++i) {
				if (m_stpClientInfo[i].m_bIsLogined) {
					HandleNPCView(i, Id);
					if (IsCloseWithNPC(Id, i)) {
						Timer_Event t = { Id, high_resolution_clock::now() + 10s,E_MOVE };
						m_tqLock.lock(); m_timer_queue.push(t); m_tqLock.unlock();
						continue;
					}
					
				}
			}
			m_cNPC[Id].SetPassive();
			delete stpOverlappedEx;

		}
			
		// Exception
		else {
			printf("socket(%d) Exception", m_stpClientInfo[Id].m_SocketClient);
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

LONGLONG CIOCP::Get_Current_Time()
{
	milliseconds ms = duration_cast< milliseconds >(system_clock::now().time_since_epoch());
	return ms.count();
}


LONGLONG CIOCP::AddTime(WORD a_wTime)
{
	milliseconds ms = duration_cast< milliseconds >(system_clock::now().time_since_epoch());
	return ms.count() + a_wTime;
}

void CIOCP::TimerThread()
{
	for (;;) {
		Sleep(10);
		for (;;) {
			m_tqLock.lock();
			if (0 == m_timer_queue.size()) {
				m_tqLock.unlock(); break;
			}
			Timer_Event t = m_timer_queue.top();
			if (t.exec_time > high_resolution_clock::now()) {
				m_tqLock.unlock(); break;
			}
			m_timer_queue.pop();
			m_tqLock.unlock();
			stOverlappedEx* over = new stOverlappedEx;
			if (E_MOVE == t.event) over->m_eOperation = eOP_DO_AI;
			PostQueuedCompletionStatus(m_hIOCP, 1, t.object_id, &over->m_wsaOverlapped);
		}
	}
}


void CIOCP::ConnectDB()
{
	m_CDB.Connect();

}

void CIOCP::WakeUpNPC(int id)
{
	if (m_cNPC[id].GetIsActivated()) return;
	m_cNPC[id].SetActive();
	Timer_Event event{id, high_resolution_clock::now() + 10s, E_MOVE};
	m_tqLock.lock(); m_timer_queue.push(event); m_tqLock.unlock();

}