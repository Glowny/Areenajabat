#include "master_server.h"
#include <bx/thread.h>
#include <common/salt.h>

namespace arena
{
    MasterServer::MasterServer()
        : m_running(false)
    {
        m_gameInstances.reserve(MaxGameInstances);
        memset(m_instanceCreatedBy, 0, sizeof(m_instanceCreatedBy));
        memset(m_lobbySalts, 0, sizeof(m_lobbySalts));
    }

    void MasterServer::start()
    {
        ARENA_ASSERT(m_running == false, "Server already running");

        uint16_t port = 8088;

        m_networkInterface = new NetworkInterface(port);

        fprintf(stderr, "Accepting connections on port %" PRIu16 "\n", port);

        m_running = true;

        while (m_running)
        {
            update();
        }
    }

    MasterServer::~MasterServer()
    {
        delete m_networkInterface;
    }

    void MasterServer::update()
    {

        // dispatch writte packets and receive from client
        ENetEvent event;
        while (enet_host_service(m_networkInterface->m_socket, &event, 0) > 0)
        {
            if (event.type == ENET_EVENT_TYPE_RECEIVE)
            {
                // this call will enqueue serialized packet to queue
                m_networkInterface->readPacket(event.peer, event.packet);
            }
            else if (event.type == ENET_EVENT_TYPE_CONNECT)
            {
                printf("ENET: connected\n");
            }
            else if (event.type == ENET_EVENT_TYPE_DISCONNECT)
            {
                printf("ENET: diconnected\n");
                // hmmm we can't close the socket now so we need to implement new system...
                // because we dont know the clientSalt nor challenge
            }
        }

        ENetPeer* from;
        Packet* pkg;
        while ((pkg = m_networkInterface->receivePacket(from)) != nullptr)
        {
            int32_t type = pkg->getType();
            if (type >= PacketTypes::MasterCreateLobby && type <= PacketTypes::MasterListLobbies)
            {
                switch (type)
                {
                    case PacketTypes::MasterCreateLobby:
                    {
                        processCreateLobbyPacket((CreateLobbyPacket*)pkg, from, 0.0);
                    }
                    break;
                    case PacketTypes::MasterJoinLobby:
                    {
                        processJoinLobbyPacket((JoinLobbyPacket*)pkg, from, 0.0);
                    }
                    break;
                    case PacketTypes::MasterListLobbies:
                    {

                    }
                    break;
                    default:
                        break;
                }
            }
            else
            {
                // route packets to correct slaves
                fprintf(stderr, "Packet of type %d needs to be routed\n", pkg->getType());
            }

            // update


            // get data from slaves
            // ....

            // write data 
            m_networkInterface->writePackets();
        }
    }

    uint64_t calculateLobbySalt(ENetPeer* peer, uint64_t clientSalt, const char* lobbyname)
    {
        char addressString[256];
        enet_address_get_host_ip(&peer->address, addressString, sizeof(addressString));
        const uint32_t len = (uint32_t)strlen(addressString);

        const uint64_t seed = murmur_hash_64(lobbyname, strlen(lobbyname), 0);

        return murmur_hash_64(
            &seed,
            8u,
            murmur_hash_64(&clientSalt, 8, murmur_hash_64(addressString, len, 0))
        );
    }

    void MasterServer::processCreateLobbyPacket(CreateLobbyPacket* packet, ENetPeer* from, double timestamp)
    {
        (void)timestamp;
        for (uint32_t i = 0; i < (uint32_t)m_gameInstances.size(); ++i)
        {
            if (m_instanceCreatedBy[i] == packet->m_clientSalt)
            {
                // already created lobby....
                return;
            }
        }

        if (m_gameInstances.size() < MaxGameInstances)
        {
            m_gameInstances.push_back(new SlaveServer());
            const uint32_t lobbyIndex = m_gameInstances.size() - 1;
            m_instanceCreatedBy[lobbyIndex] = packet->m_clientSalt;
            m_lobbySalts[lobbyIndex] = calculateLobbySalt(from, packet->m_clientSalt, packet->m_name);

            fprintf(stderr, "Created new slave (salt = %" PRIx64 ") (index = %" PRIu32 ")\n", m_lobbySalts[lobbyIndex], lobbyIndex);

            LobbyResultPacket* response = (LobbyResultPacket*)createPacket(PacketTypes::LobbyResultPacket);
            response->m_created = true;
            
            // todo if dc
            m_networkInterface->sendPacket(from, response);
        }
    }

    void MasterServer::processJoinLobbyPacket(JoinLobbyPacket* packet, ENetPeer* from, double timestamp)
    {
        BX_UNUSED(packet, from, timestamp);
    }
}