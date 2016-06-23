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
	client.start("172.31.16.46", 8888);
#else
	Server server;
	server.start(8888, 2);
#endif
}

