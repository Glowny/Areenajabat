#include "NetworkServer.h"
#include <common/Enumerations.h>
#if defined(ARENA_SERVER)
Network::Network() {}

Network::~Network()
{
	enet_host_destroy(m_server);
}
void Network::startServer(std::queue<Message>* messageQueue, unsigned address,
	unsigned port, unsigned clientAmount)
{
    m_messageQueue = messageQueue;
	idGiver = 0;

}
void Network::checkEvent() 
{
	// Does event have to persist after one loop? peer->data is saved
	ENetEvent EEvent;
	enet_host_service(m_server, &EEvent, 0);

	switch (EEvent.type)
	{
	case ENET_EVENT_TYPE_CONNECT:
	{		printf("A new client connected from %x:%u.\n",
			EEvent.peer->address.host,
			EEvent.peer->address.port);
		// dunno maybe does not need this
		int* id = new int(idGiver);
		EEvent.peer->data = (void*)id;
		idGiver++;
		m_clientVector.push_back(Client{777, EEvent.peer });
		break;
	}

	case ENET_EVENT_TYPE_RECEIVE:
	{

		// !!TODO: Data is copied because the message has to be destroyed at some point by ENet
		// This should be handled differently, reserving and freeing space twice for the data is wasteful.
		Message message;
		message.data = (unsigned char*)malloc(EEvent.packet->dataLength);
		memcpy(message.data, EEvent.packet->data, EEvent.packet->dataLength);
		
		// Make better some day
		for (unsigned i = 0; i < m_clientVector.size(); i++)
		{
			if (m_clientVector[i].peer == EEvent.peer)
			{
				message.playerID = m_clientVector[i].playerID;
				break;
			}
		};
		m_messageQueue->push(message);

		enet_packet_destroy(EEvent.packet);
		break;
	}
	case ENET_EVENT_TYPE_DISCONNECT:
	{
		unsigned index;
		for (unsigned i = 0; i < m_clientVector.size(); i++)
		{
			if (EEvent.peer == m_clientVector[i].peer)
			{
				index = i;
			}
		}

		printf("client id: %d, clientVector index: %d player id: %d disconnected. playerAmount: %d\n Deleting player\n", (unsigned)EEvent.peer->data, index, m_clientVector[index].playerID, m_clientVector.size());

		free(EEvent.peer->data);
		EEvent.peer->data = NULL;

		Message message;
		message.data = (unsigned char*)malloc(sizeof(MessageIdentifier));
		
		MessageIdentifier messageid = Restart;
		memcpy(message.data, &messageid, sizeof(messageid));
		message.playerID = 666;
		m_messageQueue->push(message);

		enet_peer_reset(EEvent.peer);
		m_clientVector.erase(m_clientVector.begin() + index);
		printf("index %d disconnected. PlayerAmount: %d\n", index, m_clientVector.size());
	}
	default:
		break;
	}

}

void Network::sendPacket(unsigned char* data, uint32_t size, 
	unsigned playerIndex, bool reliable)
{
	_ENetPacketFlag flag;
	if (reliable)
		flag = ENET_PACKET_FLAG_RELIABLE;
	else
		flag = ENET_PACKET_FLAG_UNSEQUENCED;

	ENetPacket *ePacket = enet_packet_create(data,
		size,
		flag);


	for (unsigned i = 0; i < m_clientVector.size(); i++)
	{
		if (m_clientVector[i].playerID == playerIndex)
		{
			enet_peer_send(m_clientVector[i].peer, 0, ePacket);
			enet_host_flush(m_server);
			break;
		}
	}
}

void Network::broadcastPacket(unsigned char* data, uint32_t size, bool reliable)
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

// Player id != clienindx
void Network::disconnectClient(unsigned playerIndex)
{
	unsigned clientIndex;
	for (unsigned i = 0; i < m_clientVector.size(); i++)
	{
		
		if (playerIndex == m_clientVector[i].playerID)
		{ 
			clientIndex = m_clientVector[i].playerID;
			break;
		}
	}
	enet_peer_disconnect(m_clientVector[clientIndex].peer, 0);
	// Makes server stuck.
	ENetEvent EEvent;
	while (enet_host_service(m_server, &EEvent, 3000) > 0)
		switch (EEvent.type)
		{
		case ENET_EVENT_TYPE_RECEIVE:
		{
			unsigned id = unsigned(EEvent.peer->data);
			if (id == clientIndex)
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

void Network::setPlayerIds(std::vector<unsigned int> idVector)
{
	for (unsigned i = 0; i < m_clientVector.size(); i++)
	{
		m_clientVector[i].playerID = idVector[i];
	}
}

#endif