#include "NetworkClient.h"
#if defined(ARENA_CLIENT)
Network::Network()
{

}

Network::~Network()
{
	enet_host_destroy(m_client);
	enet_deinitialize();
}

void Network::checkEvent() 
{
	// Does event have to persist after one loop? peer->data is saved
	ENetEvent EEvent;
	enet_host_service(m_client, &EEvent, 0);

	switch (EEvent.type)
	{
	case ENET_EVENT_TYPE_CONNECT:
		printf("Somebody connected here?",
			EEvent.peer->address.host,
			EEvent.peer->address.port);
		EEvent.peer->data = "Host";
		break;

	case ENET_EVENT_TYPE_RECEIVE:
	{
	
		// !!TODO: Data is copied because the message has to be destroyed at some point by ENet
		// This should be handled differently, reserving and freeing space twice for the data is wasteful.
		
		unsigned char* data = (unsigned char*)malloc(EEvent.packet->dataLength);
		memcpy(data, EEvent.packet->data, EEvent.packet->dataLength);
		m_messageQueue->push(data);

		enet_packet_destroy(EEvent.packet);
		break;
	}
	case ENET_EVENT_TYPE_DISCONNECT:
		printf("%s disconnected.\n", EEvent.peer->data);
		EEvent.peer->data = NULL;
	default:
		break;
	}

}

void Network::sendPacket(unsigned char* data, size_t size, bool reliable)
{
	_ENetPacketFlag flag;
	if (reliable)
		flag = ENET_PACKET_FLAG_RELIABLE;
	else
		flag = ENET_PACKET_FLAG_UNSEQUENCED;

	ENetPacket *ePacket = enet_packet_create(data,
		size,
		flag);
	enet_peer_send(m_peer, 0, ePacket);
	
	
	
}
// When all packets are ready to send 
void Network::sendMessages()
{
	enet_host_flush(m_client);
}


void Network::disconnect()
{
	enet_host_destroy(m_client);
	enet_peer_disconnect(m_peer, 0);
	// Makes server stuck.
	ENetEvent EEvent;
	while (enet_host_service(m_client, &EEvent, 3000) > 0)
		switch (EEvent.type)
		{
		case ENET_EVENT_TYPE_RECEIVE:
		{
			enet_packet_destroy(EEvent.packet);
			break;
		}
		case ENET_EVENT_TYPE_DISCONNECT:
			puts("Disconnection succeeded.");
			return;
		default:
			break;
		}
	enet_peer_reset(EEvent.peer);
}

void Network::setMessageQueue(std::queue<unsigned char*> *messageQueue)
{
	m_messageQueue = messageQueue;
}

void Network:: initializeENet()
{
	if (enet_initialize() != 0)
	{
		fprintf(stderr, "An error occured while intializing ENET. \n");
		abort();
	}
}

ENetHost* Network::createENetClient()
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
void Network::connectServer(char* address, unsigned port)
{
	initializeENet();
	m_client = createENetClient();
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
	ENetEvent m_eEvent;
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
#endif