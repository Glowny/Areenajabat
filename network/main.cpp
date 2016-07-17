#include "master_server/master_server.h"

#define SERVER_SANDBOX

int main(int argc, char *argv[])
{
    using namespace arena;
    BX_UNUSED(argc, argv);
    networkInitialize();
	
    MasterServer server;

	server.start();
    
    networkShutdown();
}
