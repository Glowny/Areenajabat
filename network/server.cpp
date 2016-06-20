#include "server.h"
#include <stdio.h>
#include <assert.h>
Server::Server()
{
	m_clientAmount = 0;
}

Server::~Server()
{
	// Free up resources used by enet.
	enet_host_destroy(m_server);
	enet_deinitialize();
	for (unsigned clientIndex = 0; clientIndex < m_clientAmount; clientIndex++)
	{
		enet_host_destroy(m_clientArray[clientIndex]);
	}
}

void Server::start(unsigned port, unsigned playerAmount)
{
	m_clientAmount = playerAmount;

	initializeENet();
	m_server = createENetServer(0, port, playerAmount);
	for (unsigned clientIndex = 0; clientIndex < m_clientAmount; clientIndex++)
	{
		m_clientArray[clientIndex] = createENetClient();
	}
	
}

// Help: http://enet.bespin.org/Tutorial.html#Connecting

void Server::initializeENet()
{
	if (enet_initialize() != 0)
	{
		fprintf(stderr, "An error occured while intializing ENET. \n");	
		abort();
	}
}

ENetHost* Server::createENetServer(unsigned address, unsigned port, unsigned playerAmount)
{
	ENetAddress eAddress;
	ENetHost* eServer;

	if (address == 0)
		eAddress.host = ENET_HOST_ANY;
	else
		eAddress.host = address;
	eAddress.port = port;

	eServer = enet_host_create(&eAddress /* the address to bind the server host to */,
		playerAmount      /* allow up to playerAmount clients and/or outgoing connections */,
		2      /* allow up to 2 channels to be used, 0 and 1 */,
		0      /* assume any amount of incoming bandwidth */,
		0      /* assume any amount of outgoing bandwidth */);

	if (eServer == NULL)
	{
		fprintf(stderr, "An error occured while trying to create ENet server host \n");
		abort();
	}
	return eServer;
}

ENetHost* Server::createENetClient()
{
	ENetHost* client;
	client = enet_host_create(NULL /* create a client host */,
		1 /* only allow 1 outgoing connection */,
		2 /* allow up 2 channels to be used, 0 and 1 */,
		57600 / 8 /* 56K modem with 56 Kbps downstream bandwidth */,
		14400 / 8 /* 56K modem with 14 Kbps upstream bandwidth */);

	if (client == NULL)
	{
		fprintf(stderr, "An error occured while rying to create an ENEt client host .\n");
		abort();
	}
}

char* Server::getData(unsigned clientIndex)
{
	ENetEvent EEvent;
	enet_host_service(m_clientArray[clientIndex], &EEvent, 0);
	return NULL;
}