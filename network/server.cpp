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
	
	enet_host_destroy(m_server);
	
}

void Server::start(unsigned port, unsigned playerAmount)
{
	initializeENet();
	m_server = createENetServer(0, port, playerAmount);
	int what = 0;
	while (true)
	{
		what = checkEvent(); // do check for all later
		switch (what)
		{
			case 0:
				break;
			case 1:
				printf("Client connected \n");
				break;
			case 2:
				printf("Client send a message, sending reply \n");
				sendPacket("Shut up", strlen("Shut up"), 0);
				what = 0;
				break;
			case 3:
				printf("Client disconnected\n");
				break;
		}
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


	//for (int i = 0; i < playerAmount; i++)
	//{
	//	ENetEvent EEvent;
	//	m_clientArray[playerAmount] = EEvent;
	//}

	if (eServer == NULL)
	{
		fprintf(stderr, "An error occured while trying to create ENet server host \n");
		abort();
	}
	return eServer;
}



int Server::checkEvent()
{
	// Does event have to persist after one loop? peer->data is saved
	ENetEvent EEvent;
	enet_host_service(m_server, &EEvent, 0);

	switch (EEvent.type)
	{
	case ENET_EVENT_TYPE_CONNECT:
		printf("A new client connected from %x:%u.\n",
			EEvent.peer->address.host,
			EEvent.peer->address.port);
		EEvent.peer->data = "This man is very good";
		m_clientArray[m_clientAmount] = EEvent.peer;
		m_clientAmount++;
		return 1;
		break;
	
	case ENET_EVENT_TYPE_RECEIVE:
		printf("A packet of length %u containing %s was received from %s on channel %u.\n",
			EEvent.packet->dataLength,
			EEvent.packet->data,
			EEvent.peer->data,
			EEvent.channelID);
		// save data at this point and then destory packet.
		enet_packet_destroy(EEvent.packet);
		return 2;
		break;

	case ENET_EVENT_TYPE_DISCONNECT:
		printf("%s disconnected.\n", EEvent.peer->data);
		EEvent.peer->data = NULL;
		return 3;
	}
	return 0;
}

void Server::sendPacket(char* testString, unsigned size, unsigned clientIndex)
{
	ENetPacket *packet = enet_packet_create(testString,
											size, 
											ENET_PACKET_FLAG_RELIABLE);
	enet_peer_send(m_clientArray[clientIndex], 0, packet);
	// When all packets are ready to send use
	enet_host_flush(m_server);
}

void Server::broadcastPacket(char* testString, unsigned size)
{
	ENetPacket *packet = enet_packet_create(testString,
		size,
		ENET_PACKET_FLAG_RELIABLE);
	enet_host_broadcast(m_server, 0, packet);
}

void Server::disconnectClient(unsigned clientIndex)
{
	enet_peer_disconnect(m_clientArray[clientIndex], 0);
	ENetEvent eEvent;
	// Allow client to disconnect gently for three secounds.
	// This has to be done differently later, otherwise server freezes for three seconds.
	// Timer could be started on the main loop and use force disconnect there.
	while(enet_host_service (m_server, &eEvent, 3000) > 0)
		switch (eEvent.type)
		{
		case ENET_EVENT_TYPE_RECEIVE:
			enet_packet_destroy(eEvent.packet);
			break;
		case ENET_EVENT_TYPE_DISCONNECT:
			puts("Disconnection succeeded.");
			return;
		}
	enet_peer_reset(eEvent.peer);
}