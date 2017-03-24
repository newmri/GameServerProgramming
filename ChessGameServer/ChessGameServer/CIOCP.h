#pragma once

#include "ClientInfo.h"

class CIOCP
{
public:
	CIOCP();
	~CIOCP();

	// -- Common Functions Server and Client  -- //
	bool InitSocket();
	void CloseSocket(stClientInfo*, bool bIsForce = false);

	// -- Functions for Server -- //
	bool BindandListen(int);
	bool StartServer();

	// Create worker thread to wait in WaitingThread Queue
	bool CreateWorkerThread();

	// Create accept thread
	bool CreateAccepterThread();

	// release will not use objects
	stClientInfo* GetEmptyClientInfo();

	// Connect object of completionPort and socket and completionKey
	bool BindIOCompletionPort(stClientInfo*);

	// Make doing WSARecv Overlapped I/O 
	bool BindRecv(stClientInfo*);

	// Make doing WSASend Overlapped I/O 
	bool SendMsg(stClientInfo*, char*, int);

	// Recv and  process I/O work has been completed 
	void WorkerThread();

	// Accepter Thread
	void AccepterThread();

	// Destroy constructed thread
	void DestroyThread();

private:
	// structure that have client's information 
	stClientInfo* m_stpClientInfo;
	// ListenSocket
	SOCKET m_ListenSocket;
	// Number of connected client
	int m_nClientCnt;
	// Working Thread Handle -- this will get to WaitingThread Queue
	HANDLE m_hWorkerThread[MAX_WORKER_THREAD];
	// Accept Thread Handle
	HANDLE m_hAccepterThread;
	// CompletionPort Object Handle
	HANDLE m_hIOCP;
	// Working Thread Operation Flag
	bool m_bWorkerRun;
	// Accept Thread Operation Flag
	bool m_bAccepterRun;
	// Socket Buffer
	char m_szSocketBuf[MAX_BUF_SIZE];
	// Message Buffer
	char m_szMsgbuf[MAX_MSG_SIZE + sizeof(int)];
	// return variables
	bool m_bRetval;
	bool m_bErr;
};