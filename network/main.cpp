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

	Server server;
	server.start(0, 8888, 3);
		
#endif
}

