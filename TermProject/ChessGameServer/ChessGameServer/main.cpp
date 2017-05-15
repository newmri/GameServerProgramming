#include "CIOCP.h"

int main()
{
	CIOCP* pIOCP = CIOCP::Instance();

	pIOCP->InitSocket();
	pIOCP->BindandListen(SERVER_PORT);
	pIOCP->StartServer();

	pIOCP->DestroyInstance();

}