#include <iostream>

#include "ORpcStreamServer.h"

int main()
{
	RpcServer::ORpcStreamServer server;

	server.Start(8860);

	while (1)
	{
		Sleep(1000);
	}

	return 0;
}