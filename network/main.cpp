#pragma once
#include <enet/enet.h>


#if defined(ARENA_CLIENT)
#	include "client_sandbox.h"


#else
	
#	include "server.h"
#endif

int main() 
{

	
#if defined(ARENA_CLIENT)
		
	Client client;

	client.start("172.31.16.42", 8888);

#else
    arena::networkInitialize();

	Server server(8888);
	server.start(0, 8888, 3);
	
    arena::networkShutdown();
#endif
}

