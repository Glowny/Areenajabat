#pragma once
#include <enet/enet.h>


#if defined(ARENA_CLIENT)
#	include "client_sandbox.h"


#else
	
#	include "server.h"
#endif

#define SERVER_SANDBOX

int main(int argc, char *argv[])
{
#if defined(ARENA_CLIENT)
		
	Client client;

	client.start("localhost", 8888);

#elif defined(SERVER_SANDBOX)
		Server server;
		server.start("..\\..\\..\\ini\\sandbox.ini");
#else
	if (argc == 0)
	{
		Server server;
		server.start(0, 8888, 3);
	}
	else 
	{
		Server server;
		server.start(argv[0]);
	}
#endif
}

