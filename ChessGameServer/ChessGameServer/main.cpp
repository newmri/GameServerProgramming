#include "CIOCP.h"

int main()
{
	CIOCP* pIOCP = CIOCP::Instance();

	pIOCP->InitSocket();
	pIOCP->BindandListen(9000);
	pIOCP->StartServer();

	pIOCP->DestroyInstance();

}