#include "Network.h"

Network::Network() {}

Network::~Network()
{
	enet_host_destroy(m_server);
	enet_deinitialize();

	enet_host_destroy(m_server);
}
void Network::startServer(std::queue<Message>* messageQueue, unsigned address,
	unsigned port, unsigned clientAmount)
{
	m_messageQueue = messageQueue;
	initializeENet();
	m_server = createENetServer(address, port, clientAmount);

}
void Network::checkEvent() 
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
		EEvent.peer->data = (void*)m_clientVector.size();
		m_clientVector.push_back(EEvent.peer);
		break;

	case ENET_EVENT_TYPE_RECEIVE:
	{
		printf("A packet of length %u containing %s was received from %d on channel %u.\n",
			EEvent.packet->dataLength,
			EEvent.packet->data,
			EEvent.peer->data,
			EEvent.channelID);
		unsigned id = unsigned(EEvent.peer->data);
		printf("%u\n", id);
	
		// !!TODO: Data is copied because the message has to be destroyed at some point by ENet
		// This should be handled differently, reserving and freeing space twice for the data is wasteful.
		Message message;
		message.data = (unsigned char*)malloc(EEvent.packet->dataLength);
		memcpy(message.data, EEvent.packet->data, EEvent.packet->dataLength);
		message.clientID = id;
		m_messageQueue->push(message);

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

void Network::sendPacket(unsigned char* data, size_t size, 
	unsigned clientIndex, bool reliable = true)
{
	_ENetPacketFlag flag;
	if (reliable)
		flag = ENET_PACKET_FLAG_RELIABLE;
	else
		flag = ENET_PACKET_FLAG_UNSEQUENCED;

	ENetPacket *ePacket = enet_packet_create(data,
		size,
		flag);
	enet_peer_send(m_clientVector[clientIndex], 0, ePacket);
	
	// When all packets are ready to send 
	enet_host_flush(m_server);
}

void Network::broadcastPacket(unsigned char* data, size_t size, bool reliable = true)
{
	_ENetPacketFlag flag;
	if (reliable)
		flag = ENET_PACKET_FLAG_RELIABLE;
	else
		flag = ENET_PACKET_FLAG_UNSEQUENCED;
	ENetPacket *ePacket = enet_packet_create(data,
		size,
		flag);
	enet_host_broadcast(m_server, 0, ePacket);
	enet_host_flush(m_server);
}

void Network::disconnectClient(unsigned clientIndex)
{
	enet_peer_disconnect(m_clientVector[clientIndex], 0);
	// Makes server stuck.
	ENetEvent EEvent;
	while (enet_host_service(m_server, &EEvent, 3000) > 0)
		switch (EEvent.type)
		{
		case ENET_EVENT_TYPE_RECEIVE:
			unsigned id = unsigned(EEvent.peer->data);
			if (id == clientIndex)
				enet_packet_destroy(EEvent.packet);
			break;
		case ENET_EVENT_TYPE_DISCONNECT:
			puts("Disconnection succeeded.");
			return;
		default:
			break;
		}
	enet_peer_reset(EEvent.peer);
}

void Network:: initializeENet()
{
	if (enet_initialize() != 0)
	{
		fprintf(stderr, "An error occured while intializing ENET. \n");
		abort();
	}
}

ENetHost* Network::createENetServer(unsigned address, unsigned port,
	unsigned clientAmount)
{
	ENetAddress eAddress;
	ENetHost* eServer;

	if (address == 0)
		eAddress.host = ENET_HOST_ANY;
	else
		eAddress.host = address;
	eAddress.port = port;

	eServer = enet_host_create(&eAddress /* the address to bind the server host to */,
		clientAmount      /* allow up to playerAmount clients and/or outgoing connections */,
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