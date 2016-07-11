#include "slave_server.h"
using namespace arena;

void SlaveServer::pushPacketToQueue(Packet* packet)
{
	// TODO: When threads are implemented, add mutex.
	m_outPacketQueue->push(packet);
}

Packet* SlaveServer::getPacketFromQueue()
{
	// TODO: When threads are implemented, add mutex.
	Packet* packet = m_inPacketQueue->front();
	m_inPacketQueue->pop();
	return packet;
}

void SlaveServer::handleIncomingPackets()
{
	while (!m_inPacketQueue->empty())
	{
		handleSinglePacket(getPacketFromQueue());
	}
}

void SlaveServer::handleSinglePacket(Packet* packet)
{
	switch (packet->getType())
	{
		case PacketTypes::GameInput:
		{
			GameInputPacket* inputPacket = (GameInputPacket*)packet;
			//TODO: Set ID on master server.
			inputPacket->m_id;
			inputPacket->x;
			inputPacket->y;
			break;
		}

		case PacketTypes::GameShoot:
		{
			GameShootPacket* shootPacket = (GameShootPacket*)packet;
			shootPacket->m_id;
			shootPacket->m_angle;
			break;
		}

		default:
			printf("Game: Packet id: %d unknown.\n", packet->getType());
			break;
	}
}