#include "master_server.h"


namespace arena
{
    void MasterServer::update()
    {
        ENetPeer* from;
        Packet* pkg;
        while ((pkg = m_networkInterface->receivePacket(from)) != nullptr)
        {
            if (pkg->getType() > PacketTypes::Disconnect)
            {
                //uint64_t clientSalt = *(uint64_t*)from->data;
                //m_serverLookup[clientSalt]->
            }
            // matchmaker
            else
            {
                

            }
        }
    }
}