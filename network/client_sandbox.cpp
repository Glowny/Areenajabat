#include "client_sandbox.h"
void Client::start(char* address, unsigned port)
{
	initializeENet();
	m_client = createENetClient();
	connect(address, port);

	float deltaTime = 0;
	while (true)
	{
		checkEvent();
		deltaTime += 0.01;
		if (deltaTime > 100000)
		{ 
			printf("Sending packet \n");
			sendPacket("AA KESaN LAPSI Ma OON", strlen("AA KESaN LAPSI Ma OON"));
			deltaTime = 0;
		}
	}

}

void Client::initializeENet()
{
	if (enet_initialize() != 0)
	{
		fprintf(stderr, "An error occured while intializing ENET. \n");
		abort();
	}
}

ENetHost* Client::createENetClient()
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
	return client;
}
void Client::checkEvent()
{
	ENetEvent EEvent;
	enet_host_service(m_client, &EEvent, 0);

	switch (EEvent.type)
	{
	case ENET_EVENT_TYPE_CONNECT:
		printf("Somebody connected here?",
			EEvent.peer->address.host,
			EEvent.peer->address.port);
		EEvent.peer->data = "This man is very good";
		break;

	case ENET_EVENT_TYPE_RECEIVE:
		printf("A packet of length %u containing %s was received from %s on channel %u.\n",
			EEvent.packet->dataLength,
			EEvent.packet->data,
			EEvent.peer->data,
			EEvent.channelID);
		// save data at this point and then destory packet.
		enet_packet_destroy(EEvent.packet);
		break;

	case ENET_EVENT_TYPE_DISCONNECT:
		printf("%s disconnected.\n", EEvent.peer->data);
		EEvent.peer->data = NULL;
	}

}
void Client::connect(char* address, unsigned port)
{
	ENetAddress eAddress;
	enet_address_set_host(&eAddress, address);
	eAddress.port = port;

	m_peer = enet_host_connect(m_client, &eAddress, 2, 0);
	if (m_peer == NULL)
	{
		fprintf(stderr,
			"No available peers for initiating an ENet connection.\n");
		abort();
	}

	if (enet_host_service(m_client, &m_eEvent, 5000) > 0 &&
		m_eEvent.type == ENET_EVENT_TYPE_CONNECT)
	{
		printf("Connection to %s,: %u succeeded", address, &port);
	}
	else
	{
		enet_peer_reset(m_peer);
		printf("Connection to %s,: %u failed", address, &port);
	}
}

void Client::sendPacket(char* testString, unsigned size)
{
	ENetPacket *packet = enet_packet_create(testString,
		size,
		ENET_PACKET_FLAG_RELIABLE);
	enet_peer_send(m_peer, 0, packet);
	// When all packets are ready to send use
	enet_host_flush(m_client);
}