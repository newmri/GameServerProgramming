#include "CIOCP.h"

int main()
{
	CIOCP cIocp;
	cIocp.InitSocket();
	cIocp.BindandListen(9000);
	cIocp.StartServer();

}