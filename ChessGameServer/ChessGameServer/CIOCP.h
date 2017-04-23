#pragma once

#include "ClientInfo.h"

class CIOCP
{
public:
	~CIOCP();

	// -- Common Functions Server and Client  -- //
	bool InitSocket();
	void CloseSocket(const WORD&, const bool& bIsForce = false);

	// -- Functions for Server -- //
	const bool& BindandListen(const int&);
	void StartServer();

	// Set new client info
	void SetNewClientInfo(const WORD&);

	const bool& IsClose(const WORD&, const WORD&);

	const bool& CreateWorkerThread();

	const bool& CreateAccepterThread();

	// Connect object of completionPort and socket and completionKey
	const bool& BindAndRecvIOCompletionPort(const WORD&);

	// Recv and  process I/O work has been completed 
	void WorkerThread();

	// Handle Error
	void DisPlayError(const char*, const int&);

	void HandleView(const WORD&);

	void ProcessPacket(const WORD&, const unsigned char[]);

	void SendPacket(const WORD&, void*);

	void SendPutClient(const WORD&, const WORD&);

	void SendMoveClient(const WORD&, const WORD&);

	void SendRemoveClient(const WORD&, const WORD&);

	// Accepter Thread
	void AccepterThread();

	// Destroy constructed thread
	void DestroyThread();

// To contrcut only one IOCP
public:
	static CIOCP* Instance();
	static void DestroyInstance();

private:
	CIOCP();

private:
	static CIOCP* m_pInstance;

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
	// Buffer Len
	int m_nBufLen;
	// Pos to avoid collision
	POINT m_pos;
	// User Id
	unsigned short m_wId;
};