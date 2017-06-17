#include "CIOCP.h"
#include "Timer.h"
#include <process.h>

CIOCP* CIOCP::m_pInstance = NULL;

#define IOCP CIOCP::GetInstance()

bool cmp(const Point& a, const Point& b)
{
	return a.m_wZone < b.m_wZone;
}

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


	WORD compare{ 10 };
	Point2 pos2{ 0,0,1 };
	WORD Multi{1};
	for (WORD i = 0; i < MAX_NORMAL_FIXED_NIGHT; ++i) {
		if (i == compare) {
			compare += 10;
			++pos2.m_wX;
			++pos2.m_wZone;
		}

		m_cNPC[i].Init(i, pos2);

		if (compare % 160 == 0) {
			if (pos2.m_wX != 0) {
				pos2.m_wX = 0;
				pos2.m_wY += 1;
				pos2.m_wZone = 16;
			}
		}
	}
	pos2.m_wX = 0; pos2.m_wZone = 16;
	for (WORD i = MAX_NORMAL_FIXED_NIGHT; i < MAX_NPC_NUM; ++i) {
		if (i == compare + 10) {
			compare += 10;
			++pos2.m_wX;
			++pos2.m_wZone;
		}

		m_cNPC[i].Init(i, pos2);

	}
	srand((unsigned int)time(NULL));

	Point pos;
	WORD wZone{};
	for (int y = 0; y < MAX_MAP_Y / MAX_MAP_TILE; ++y) {
		for (int x = 0; x <  MAX_MAP_X / MAX_MAP_TILE; ++x) {
			pos.m_wX = rand() % (MAX_MAP_TILE - 1) + (x * MAX_MAP_TILE); // 0 ~ 19, 20 ~ 39 and So on
			pos.m_wY = rand() % (MAX_MAP_TILE - 1) + (y * MAX_MAP_TILE); // 0 ~ 19, 20 ~ 39 and So on

			while (!((pos.m_wX < ((x + 1) * MAX_MAP_TILE)) && (pos.m_wX >(x * MAX_MAP_TILE)))) {
				pos.m_wX = rand() % (MAX_MAP_TILE - 1) + (x * MAX_MAP_TILE); // 0 ~ 19, 20 ~ 39 and So on
				pos.m_wY = rand() % (MAX_MAP_TILE - 1) + (y * MAX_MAP_TILE); // 0 ~ 19, 20 ~ 39 and So on
			}
		
			pos.m_wZone = ++wZone;
			m_MapInfo.push_back(pos);
			
		}
		
	}
	sort(m_MapInfo.begin(), m_MapInfo.end(), cmp);
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

	m_cDB.Update(m_stpClientInfo[a_wId].m_Info);

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
	m_stpClientInfo[a_wNewId].m_Info.m_pos.m_wX = CHESS_FIRST_X;
	m_stpClientInfo[a_wNewId].m_Info.m_pos.m_wY = CHESS_FIRST_Y;
	m_stpClientInfo[a_wNewId].m_Info.m_pos.m_wZone = 1;
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
	return m_stpClientInfo[a_wFrom].m_Info.m_pos.m_wZone == m_stpClientInfo[a_wTo].m_Info.m_pos.m_wZone;
}

const bool& CIOCP::IsCloseWithNPC(const WORD& a_wFrom, const WORD& a_wTo)
{
	return  m_cNPC[a_wFrom].GetPos().m_wZone == m_stpClientInfo[a_wTo].m_Info.m_pos.m_wZone;

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
	ST_SC_LOGIN_FAIL stPacket;
	stPacket.m_bytSize = sizeof(stPacket);
	stPacket.m_bytType = a_login;

	SendPacket(a_wClient, &stPacket);

}
void CIOCP::SendLoginSuccess(const WORD& a_wClient)
{
	ST_SC_LOGIN_SUCCESS stPacket;
	stPacket.m_bytSize = sizeof(stPacket);
	stPacket.m_bytType = eSC_LOGIN_SUCCESS;
	memcpy(&stPacket.m_DBInfo, &m_stpClientInfo[a_wClient].m_Info, sizeof(m_stpClientInfo[a_wClient].m_Info));
	SendPacket(a_wClient, &stPacket);
}
void CIOCP::LoginSuccessProcess(const WORD& wNewId)
{
	m_stpClientInfo[wNewId].m_bIsLogined = true;

	SendLoginSuccess(wNewId);

	SendPutClient(wNewId, wNewId);
	SendNotiFyMap(wNewId);

	unordered_set<WORD> local_view_list;
	unordered_set<WORD> local_NPC_view_list;

	// Client view
	for (int i = 0; i < MAX_CLIENT_NUM; ++i) {
		if (m_stpClientInfo[i].m_bIsLogined)
			if (i != wNewId) {
				if (IsClose(i, wNewId)) {
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
			m_cNPC[i].WakeUp();
			local_NPC_view_list.insert(i);
		}
	}

	m_stpClientInfo[wNewId].m_NPC_Lock.lock();
	for (auto p : local_NPC_view_list) m_stpClientInfo[wNewId].m_NPC_view_list.insert(p);
	m_stpClientInfo[wNewId].m_NPC_Lock.unlock();


}
void CIOCP::SendNotiFyMap(const WORD& a_wClient)
{
	ST_SC_NOTIFY_MAP stPacket;
	stPacket.m_bytSize = sizeof(stPacket);
	stPacket.m_bytType = eSC_MAP_NOTIFY;

	for (auto d : m_MapInfo) {
		memcpy(&stPacket.m_pos,&d,sizeof(Point));
		SendPacket(a_wClient, &stPacket);
	}
}

void CIOCP::SendPutClient(const WORD& a_wClient, const WORD& a_wObject)
{
	ST_SC_PUT_OBJECT stPacket;
	stPacket.m_wId = a_wObject;
	stPacket.m_bytSize = sizeof(stPacket);
	stPacket.m_bytType = eSC_PUT_CLIENT;
	stPacket.m_pos  = m_stpClientInfo[a_wObject].m_Info.m_pos;

	SendPacket(a_wClient, &stPacket);
}


void CIOCP::SendMoveClient(const WORD& a_wClient, const WORD& a_wObject)
{
	ST_SC_MOVE_OBJECT stPacket;
	stPacket.m_wId = a_wObject;
	stPacket.m_bytSize = sizeof(stPacket);
	stPacket.m_bytType = eSC_MOVE_CLIENT;
	stPacket.m_pos = m_stpClientInfo[a_wObject].m_Info.m_pos;

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
	stPacket.m_pos.m_wX = m_cNPC[a_wObject].GetPos().m_wX;
	stPacket.m_pos.m_wY = m_cNPC[a_wObject].GetPos().m_wY;
	stPacket.m_pos.m_wZone = m_cNPC[a_wObject].GetPos().m_wZone;

	SendPacket(a_wClient, &stPacket);
}

void CIOCP::SendMoveNPC(const WORD& a_wClient, const WORD& a_wObject)
{
	ST_SC_MOVE_OBJECT stPacket;
	stPacket.m_wId = a_wObject;
	stPacket.m_bytSize = sizeof(stPacket);
	stPacket.m_bytType = eSC_MOVE_NPC;
	stPacket.m_pos.m_wX = m_cNPC[a_wObject].GetPos().m_wX;
	stPacket.m_pos.m_wY = m_cNPC[a_wObject].GetPos().m_wY;
	stPacket.m_pos.m_wZone = m_cNPC[a_wObject].GetPos().m_wZone;

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
			} else m_stpClientInfo[target].m_lock.unlock();
		}
	}

	m_stpClientInfo[a_wId].m_lock.lock();
	for (auto p : vlc) m_stpClientInfo[a_wId].m_view_list.insert(p);
	for (auto d : removed_id_list) m_stpClientInfo[a_wId].m_view_list.erase(d);
	m_stpClientInfo[a_wId].m_lock.unlock();

	
	// HandleNPCView
	
	for (WORD i = 0; i < MAX_NPC_NUM; ++i) {
	if (IsCloseWithNPC(i, a_wId)) HandleNPCView(a_wId, i);
	}
	
}

void CIOCP::HandleNPCView(const WORD& a_wId, const WORD& a_NPC)
{
	unordered_set<WORD> new_view_list;

	// Object to be added
	unordered_set<WORD> vlc;
	// Object to be removed
	unordered_set<WORD> removed_id_list;

	if (m_stpClientInfo[a_wId].m_NPC_view_list.count(a_NPC) == 0)
	if (IsCloseWithNPC(a_NPC, a_wId)) new_view_list.insert(a_NPC);


	m_stpClientInfo[a_wId].m_NPC_Lock.lock();
	vlc = m_stpClientInfo[a_wId].m_NPC_view_list;
	m_stpClientInfo[a_wId].m_NPC_Lock.unlock();

	for (auto target : new_view_list) {
		// If Client doesn't have target NPC
		if (vlc.count(target) == 0) {
			m_cNPC[a_NPC].WakeUp();
			vlc.insert(target);
		}

		else {  // Client has target NPC
			SendMoveNPC(a_wId, a_NPC);
		}
	}

			// Now object is far away...
	for (auto target : vlc) {
		if (vlc.count(target) == 0) { // If target is far away
			SendRemoveNPC(a_wId, target);
			removed_id_list.insert(target);
		}
	}
	m_stpClientInfo[a_wId].m_NPC_Lock.lock();
	for (auto p : vlc) m_stpClientInfo[a_wId].m_NPC_view_list.insert(p);
	for (auto d : removed_id_list) m_stpClientInfo[a_wId].m_NPC_view_list.erase(d);
	m_stpClientInfo[a_wId].m_NPC_Lock.unlock();
		
}

void CIOCP::LoginProcess(const WORD& a_wId, const unsigned char a_Packet[])
{
	char ID[ID_LEN];
	char PWD[PWD_LEN];
	memcpy(ID, (const void*)&a_Packet[4], a_Packet[2]);
	memcpy(PWD, (const void*)&a_Packet[4 + ID_LEN], a_Packet[3]);
	ID[a_Packet[2]] = '\0';
	PWD[a_Packet[3]] = '\0';

	for (int i = 0; i < MAX_CLIENT_NUM; ++i) {
		if (strcmp(ID, m_stpClientInfo[i].m_Info.ID) == 0 && m_stpClientInfo[i].m_bIsLogined) {
			SendLoginFail(a_wId, LOGIN::eSC_LOGIN_FAIL_LOGINED);
			return;
		}
	}
	DBInfo Info = m_cDB.Login(ID, PWD);

	if (Info.m_pos.m_wX == DEFAULT) SendLoginFail(a_wId, LOGIN::eSC_LOGIN_FAIL_INCORRECT);
	else {
		memcpy(&m_stpClientInfo[a_wId].m_Info, &Info,sizeof(Info));
		LoginSuccessProcess(a_wId);
	}

}

void CIOCP::SendSignUpResult(const WORD& a_wClient, const SIGNUP a_signup)
{
	ST_SC_SIGNUP_RESULT stPacket;
	stPacket.m_bytSize = sizeof(stPacket);
	stPacket.m_bytType = a_signup;
	SendPacket(a_wClient, &stPacket);
}
void CIOCP::SignUpProcess(const WORD& a_wId, const unsigned char a_Packet[])
{
	char ID[ID_LEN];
	char PWD[PWD_LEN];
	memcpy(ID, (const void*)&a_Packet[4], a_Packet[2]);
	memcpy(PWD, (const void*)&a_Packet[4 + ID_LEN], a_Packet[3]);
	ID[a_Packet[2]] = '\0';
	PWD[a_Packet[3]] = '\0';

	if (m_cDB.SignUp(ID, PWD)) SendSignUpResult(a_wId, SIGNUP::eSC_SIGNUP_SUCCESS);
	else  SendSignUpResult(a_wId, SIGNUP::eSC_SIGNUP_FAIL);


}

void CIOCP::ChatProcess(const WORD& a_wId, const unsigned char a_Packet[])
{
	ST_SC_CHAT stPacket;
	stPacket.m_bytSize = sizeof(stPacket);
	stPacket.m_bytType = eSC_CHAT;
	strcpy(stPacket.m_Message, m_stpClientInfo[a_wId].m_Info.ID);
	strcat(stPacket.m_Message, ": ");
	strncat(stPacket.m_Message, (const char*)&a_Packet[3], a_Packet[2]);
	strcat(stPacket.m_Message, "\0");

	for (int i = 0; i < MAX_CLIENT_NUM; ++i) {
		if (m_stpClientInfo[i].m_Info.m_pos.m_wZone == m_stpClientInfo[a_wId].m_Info.m_pos.m_wZone && m_stpClientInfo[i].m_bIsLogined)
			SendPacket(i, &stPacket);
		
	}

}

void CIOCP::ProcessPacket(const WORD& a_wId, const unsigned char a_Packet[])
{
	switch (a_Packet[1]) {
	case eCS_LOGIN: LoginProcess(a_wId, a_Packet); return;
	case eCS_SIGNUP: SignUpProcess(a_wId, a_Packet); return;
	case eCS_CHAT: ChatProcess(a_wId, a_Packet); return;
	case eCS_UP: 
		if (m_stpClientInfo[a_wId].m_Info.m_pos.m_wY > eTOP_END)
			if (m_stpClientInfo[a_wId].m_Info.m_pos.m_wX != m_MapInfo[m_stpClientInfo[a_wId].m_Info.m_pos.m_wZone - 1].m_wX
				|| m_stpClientInfo[a_wId].m_Info.m_pos.m_wY - 1 != m_MapInfo[m_stpClientInfo[a_wId].m_Info.m_pos.m_wZone - 1].m_wY) {
			m_stpClientInfo[a_wId].m_Info.m_pos.m_wY--;
		if (m_stpClientInfo[a_wId].m_Info.m_pos.m_wYZone > -1 && m_stpClientInfo[a_wId].m_Info.m_pos.m_wY != 38) {
			if (m_stpClientInfo[a_wId].m_Info.m_pos.m_wY % (((((m_stpClientInfo[a_wId].m_Info.m_pos.m_wYZone + 1) * 16) - m_stpClientInfo[a_wId].m_Info.m_pos.m_wYZone) - ((m_stpClientInfo[a_wId].m_Info.m_pos.m_wYZone * (MAX_MAP_Y_ZONE - 1)) + MAX_MAP_Y_ZONE)) * MAX_MAP_TILE - 1) == 0) {
				m_stpClientInfo[a_wId].m_Info.m_pos.m_wZone -= MAX_MAP_Y_ZONE;
				m_stpClientInfo[a_wId].m_Info.m_pos.m_wYZone--;
			}

		}
	}break;
	case eCS_DOWN: 
		if (m_stpClientInfo[a_wId].m_Info.m_pos.m_wY < eBOTTOM_END)
			if (m_stpClientInfo[a_wId].m_Info.m_pos.m_wX != m_MapInfo[m_stpClientInfo[a_wId].m_Info.m_pos.m_wZone - 1].m_wX
				|| m_stpClientInfo[a_wId].m_Info.m_pos.m_wY + 1 != m_MapInfo[m_stpClientInfo[a_wId].m_Info.m_pos.m_wZone - 1].m_wY) {
		m_stpClientInfo[a_wId].m_Info.m_pos.m_wY++;
		if (m_stpClientInfo[a_wId].m_Info.m_pos.m_wY % MAX_MAP_TILE == 0) {
			m_stpClientInfo[a_wId].m_Info.m_pos.m_wZone += MAX_MAP_Y_ZONE;
			m_stpClientInfo[a_wId].m_Info.m_pos.m_wYZone++;
		}
	}break;
	case eCS_LEFT: 
		if (m_stpClientInfo[a_wId].m_Info.m_pos.m_wX > eLEFT_END)
			if (m_stpClientInfo[a_wId].m_Info.m_pos.m_wX - 1 != m_MapInfo[m_stpClientInfo[a_wId].m_Info.m_pos.m_wZone - 1].m_wX
				|| m_stpClientInfo[a_wId].m_Info.m_pos.m_wY != m_MapInfo[m_stpClientInfo[a_wId].m_Info.m_pos.m_wZone - 1].m_wY) {
			m_stpClientInfo[a_wId].m_Info.m_pos.m_wX--;
		if(m_stpClientInfo[a_wId].m_Info.m_pos.m_wZone >  eLEFT_END + 1 && m_stpClientInfo[a_wId].m_Info.m_pos.m_wX != 38)
			if (m_stpClientInfo[a_wId].m_Info.m_pos.m_wX % (((m_stpClientInfo[a_wId].m_Info.m_pos.m_wXZone - 1) * (MAX_MAP_TILE)) - 1) == 0) {
				if ((((m_stpClientInfo[a_wId].m_Info.m_pos.m_wXZone - 1) * (MAX_MAP_TILE)) - 1) > 0) {
					m_stpClientInfo[a_wId].m_Info.m_pos.m_wZone--;
					m_stpClientInfo[a_wId].m_Info.m_pos.m_wXZone--;
				}

			}
	}break;
	case eCS_RIGHT: 
		if (m_stpClientInfo[a_wId].m_Info.m_pos.m_wX < eRIGHT_END)
			if(m_stpClientInfo[a_wId].m_Info.m_pos.m_wX + 1 != m_MapInfo[m_stpClientInfo[a_wId].m_Info.m_pos.m_wZone - 1].m_wX
			|| m_stpClientInfo[a_wId].m_Info.m_pos.m_wY != m_MapInfo[m_stpClientInfo[a_wId].m_Info.m_pos.m_wZone - 1].m_wY){
		m_stpClientInfo[a_wId].m_Info.m_pos.m_wX++;
		if (m_stpClientInfo[a_wId].m_Info.m_pos.m_wX % MAX_MAP_TILE == 0) {
			m_stpClientInfo[a_wId].m_Info.m_pos.m_wZone++;
			m_stpClientInfo[a_wId].m_Info.m_pos.m_wXZone++;
		}
	}break;
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
			(PULONG_PTR)&Id, // CompletionKey
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

		else if (stpOverlappedEx->m_eOperation == OP_DO_AI) {
			m_cNPC[Id].Move();
			for (int i = 0; i < MAX_CLIENT_NUM; ++i) {
				if (true == m_stpClientInfo[i].m_bIsLogined) {
					m_stpClientInfo[i].m_NPC_Lock.lock();
					if (0 != m_stpClientInfo[i].m_NPC_view_list.count(Id)) {
						if (true == IsCloseWithNPC(Id, i)) {
							m_stpClientInfo[i].m_NPC_Lock.unlock();
							SendPutNPC(i, Id);
						}
						else {
							m_stpClientInfo[i].m_NPC_view_list.erase(Id);
							m_stpClientInfo[i].m_NPC_Lock.unlock();
							SendRemoveNPC(i, Id);
						}
					}
					else {
						if (true == IsCloseWithNPC(Id, i)) {
							m_stpClientInfo[i].m_NPC_view_list.insert(Id);
							m_stpClientInfo[i].m_NPC_Lock.unlock();
							SendPutNPC(i, Id);
						}
						else m_stpClientInfo[i].m_NPC_Lock.unlock();

					}
				}
			}
			bool temp{ false };
			for (int i = 0; i < MAX_CLIENT_NUM; ++i) {
				if (true == m_stpClientInfo[i].m_bIsLogined && IsCloseWithNPC(Id, i)) {
					Timer_Event t = { Id, high_resolution_clock::now() + 1s,E_MOVE };
					TIMER->tq_lock.lock(); TIMER->timer_queue.push(t); TIMER->tq_lock.unlock();
					if (!temp) temp = true;
					break;
				}
			}
			if(!temp) m_cNPC[Id].SetPasive();
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



void CIOCP::TimerThread()
{
	for (;;) {
		Sleep(10);
		for (;;) {
			TIMER->tq_lock.lock();
			if (0 == TIMER->timer_queue.size()) {
				TIMER->tq_lock.unlock(); break;
			}
			Timer_Event t = TIMER->timer_queue.top();
			if (t.exec_time > high_resolution_clock::now()) {
				TIMER->tq_lock.unlock(); break;
			}
			TIMER->timer_queue.pop();
			TIMER->tq_lock.unlock();
			stOverlappedEx* over = new stOverlappedEx;
			if (E_MOVE == t.event) over->m_eOperation = OP_DO_AI;
			PostQueuedCompletionStatus(m_hIOCP, 1, t.object_id, &over->m_wsaOverlapped);
		}
	}

}