#include "master_server.h"

//TODO: Add map for finding correct server
using namespace arena;

void MasterServer::addPacketToQueue(unsigned slaveId, Packet* packet)
{
	//TODO: When threads are implemented, add mutexes.
	m_slaves[slaveId]->m_inPacketQueue->push(packet);
}

Packet* MasterServer::getPacketFromQueue(unsigned slaveId)
{
	//TODO: When threads are implemented, add mutexes.
	Packet* packet = m_slaves[slaveId]->m_outPacketQueue->front();
	m_slaves[slaveId]->m_outPacketQueue->pop();
	return packet;
}