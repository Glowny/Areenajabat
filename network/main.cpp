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
	client.start("localhost", 8888);
#else
	Server server;
	server.start(0, 8888, 4);
#endif
}

