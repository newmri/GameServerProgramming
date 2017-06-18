//#include <vld.h>
#include "CIOCP.h"

#define _CRTDBG_MAP_ALLOC
#include <cstdlib>
#include <crtdbg.h>
#include <cstdio>

int main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//_CrtSetBreakAlloc(153);
	CIOCP* pIOCP = CIOCP::Instance();
	char* bu = new char;
	pIOCP->InitSocket();
	pIOCP->ConnectDB();
	pIOCP->BindandListen(SERVER_PORT);
	pIOCP->StartServer();

	pIOCP->DestroyInstance();
	_CrtDumpMemoryLeaks();
	system("pause");
}