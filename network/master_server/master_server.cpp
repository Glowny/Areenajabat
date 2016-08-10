#include "master_server.h"
#include <bx/thread.h>
#include <common/salt.h>
#include <bx/thread.h>


namespace arena
{
    MasterServerClientListener::MasterServerClientListener(MasterServer* master) 
        : m_master(master)
    {

    }

    MasterServerClientListener::~MasterServerClientListener()
    {

    }

    void MasterServerClientListener::onClientConnected(uint32_t clientIndex, ENetPeer* from, double timestamp)
    {
        BX_UNUSED(clientIndex, from, timestamp);
    }

    void MasterServerClientListener::onClientDisconnected(uint32_t clientIndex, ENetPeer* from, double timestamp)
    {
        fprintf(stderr, "MasterServerClientListener::onClientDisconnect()\n");

        BX_UNUSED(clientIndex, from, timestamp);
        // cut the link between lobbies and routing
        from->data = nullptr;
    }
    

    MasterServer::MasterServer() : 
        m_running(false),
        m_socket(nullptr),
        m_listener(this)
    {
        m_gameInstances.reserve(MaxGameInstances);
        memset(m_instanceCreatedBy, 0, sizeof(m_instanceCreatedBy));
        memset(m_lobbySalts, 0, sizeof(m_lobbySalts));
    }

    void MasterServer::start()
    {
        ARENA_ASSERT(m_socket == nullptr, "Socket has been already initialized");
        ARENA_ASSERT(m_running == false, "Server already running");

        uint16_t port = 8088;

        ENetAddress address;
        address.host = ENET_HOST_ANY;
        address.port = port;

        const uint32_t maxPeers = MaxGameInstances * Server::MaxClients;
        const uint32_t channelsPerPeer = 2;

        m_socket = enet_host_create(&address, maxPeers, channelsPerPeer, 0, 0);

        ARENA_ASSERT(m_socket != nullptr, "Failed to create socket");

        m_networkInterface = new NetworkInterface(m_socket);

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
            if (type >= PacketTypes::MasterCreateLobby && type < PacketTypes::GameSetup)
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
                        processListLobbiesPacket((ListLobbiesPacket*)pkg, from, 0.0);
                    }
                    break;
                    default:
                        // if client sends shit packets which are only send from server we can just drop them
                        destroyPacket(pkg);
                        break;
                }
            }
            else
            {
                // okay, the auth clients user data is never null but if someone tries to send 
                // data before joining lobby the server will crash if we try to cast data
                // to uint32_t, so just drop this packet
                if (from->data == nullptr) {
                    char ip[256];
                    enet_address_get_host_ip(&from->address, ip, sizeof(ip));
                    
                    fprintf(stderr, "!!!!! Dropping packet from %s, invalid state (not in lobby) trying to send lobby packets of type %d\n", ip, pkg->getType());

                    destroyPacket(pkg);
                    continue;
                }

                using LobbyIndex = LobbySaltToIndexMap::value_type::second_type;
                
                LobbyIndex lobbyIndex = *(LobbyIndex*)from->data;

                ARENA_ASSERT(lobbyIndex < m_gameInstances.size(), "Lobby index out of bounds, got %d", lobbyIndex);

                //fprintf(stderr, "Routing packet to slave (idx = %d, salt = %" PRIx64 ")\n", lobbyIndex, m_lobbySalts[lobbyIndex]);

                ARENA_ASSERT(m_gameInstances[lobbyIndex] != nullptr, "The game instance %d is nullptr", lobbyIndex);

                m_gameInstances[lobbyIndex]->queueIncoming(pkg, from);

                // route packets to correct slaves

                //fprintf(stderr, "Packet of type %d needs to be routed\n", pkg->getType());
            }
        }

        // update
        /*
        bx::Thread thread;
        thread.init([](void* user) {
            Server* srv = (Server*)user;
            BX_UNUSED(srv);
            printf("Thread\n");
            return EXIT_SUCCESS;
        });*/

        // TODO run paraller
        for (auto* instance : m_gameInstances)
        {
            instance->step();
        }

        // get data from slaves, this may be slow asf
        for (auto* instance : m_gameInstances)
        {
            auto& send = instance->getSendQueue();

            for (auto& entry : send)
            {
                m_networkInterface->sendPacket(entry.m_peer, entry.m_packet);
            }

            send.clear();
        }

        // write data 
        m_networkInterface->writePackets();
    }

    uint64_t calculateLobbySalt(ENetPeer* peer, uint64_t clientSalt, const char* lobbyname)
    {
        char addressString[256];
        enet_address_get_host_ip(&peer->address, addressString, sizeof(addressString));
        const uint32_t len = (uint32_t)strlen(addressString);

        const uint64_t seed = murmur_hash_64(lobbyname, (uint32_t)strlen(lobbyname), 0);

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
			// TODO: we need to know what gamemode is 
			//		 being played

			// TODO: debug
			const char* const GamemodeName = "assets/ini/dm";

            m_gameInstances.push_back(new SlaveServer(GamemodeName));

            const uint32_t lobbyIndex = uint32_t(m_gameInstances.size()) - 1;
            m_gameInstances[lobbyIndex]->addListener(&m_listener);
            m_instanceCreatedBy[lobbyIndex] = packet->m_clientSalt;

            uint64_t lobbySalt = calculateLobbySalt(from, packet->m_clientSalt, packet->m_name);
            m_lobbySalts[lobbyIndex] = lobbySalt;
            m_lobbyNames[lobbyIndex] = std::string(packet->m_name);
            m_lobbySaltToLobbyIndex[lobbySalt] =  lobbyIndex;

            fprintf(stderr, "Created new slave (salt = %" PRIx64 ") (index = %" PRIu32 ")\n", m_lobbySalts[lobbyIndex], lobbyIndex);

            LobbyResultPacket* response = (LobbyResultPacket*)createPacket(PacketTypes::LobbyResultPacket);
            response->m_created = true;
            response->m_lobbySalt = lobbySalt;

            // todo if dc
            m_networkInterface->sendPacket(from, response);
        }
        else
        {
            LobbyResultPacket* response = (LobbyResultPacket*)createPacket(PacketTypes::LobbyResultPacket);
            response->m_created = false;
            response->m_lobbySalt = 0;

            // todo if dc
            m_networkInterface->sendPacket(from, response);
        }
    }

    void MasterServer::processJoinLobbyPacket(JoinLobbyPacket* packet, ENetPeer* from, double timestamp)
    {   
        // the lobby exist
        if (m_lobbySaltToLobbyIndex.count(packet->m_lobbySalt) > 0)
        {
            // if the player is not already there
            if (from->data == nullptr)
            {
                const uint32_t lobbyIndex = m_lobbySaltToLobbyIndex[packet->m_lobbySalt];

                fprintf(stderr, "Assing client (%" PRIx64 ") to lobby (%d, salt %" PRIx64 ")\n", packet->m_clientSalt, lobbyIndex, packet->m_lobbySalt);

                LobbyJoinResultPacket* response = (LobbyJoinResultPacket*)createPacket(PacketTypes::LobbyJoinResult);
                response->m_clientSalt = packet->m_clientSalt;
                response->m_joined = true;

                m_networkInterface->sendPacket(from, response);

                // assign lobby index to peer user data
                // ATTENTION this may be hax, but it's faster than look up in map
                from->data = &m_lobbySaltToLobbyIndex[packet->m_lobbySalt];
            }
            else
            {
                fprintf(stderr, "Client (%" PRIx64 ") tried to join lobby (%" PRIx64 ") but is already joined\n", packet->m_clientSalt, packet->m_lobbySalt);
                // response to client that no no
                LobbyJoinResultPacket* response = (LobbyJoinResultPacket*)createPacket(PacketTypes::LobbyJoinResult);
                response->m_clientSalt = packet->m_clientSalt;
                response->m_joined = false;
                response->m_reason = LobbyJoinResultPacket::Reason::AlreadyJoined;

                m_networkInterface->sendPacket(from, response);
            }
        }
        else
        {
            fprintf(stderr, "Client (%" PRIx64 ") tried to join lobby (%" PRIx64 ") which doesn't exist\n", packet->m_clientSalt, packet->m_lobbySalt);
            // response to client that no no
            LobbyJoinResultPacket* response = (LobbyJoinResultPacket*)createPacket(PacketTypes::LobbyJoinResult);
            response->m_clientSalt = packet->m_clientSalt;
            response->m_joined = false;
            response->m_reason = LobbyJoinResultPacket::Reason::LobbyDoesNotExist;

            m_networkInterface->sendPacket(from, response);
        }

        

        BX_UNUSED(packet, from, timestamp);
    }

    void MasterServer::processListLobbiesPacket(ListLobbiesPacket* packet, ENetPeer* from, double timestamp)
    {
        char buf[256];
        enet_address_get_host_ip(&from->address, buf, sizeof(buf));
        fprintf(stderr, "Sending lobby listing to %s\n", buf);

        BX_UNUSED(timestamp);
        LobbyQueryResultPacket* response = (LobbyQueryResultPacket*)createPacket(PacketTypes::LobbyQueryResultPacket);
        response->m_clientSalt = packet->m_clientSalt;
        response->m_lobbyCount = (int32_t)m_gameInstances.size();
        for (uint32_t i = 0; i < (int32_t)m_gameInstances.size(); ++i)
        {
            response->m_lobbySalt[i] = m_lobbySalts[i];
            memcpy(response->m_lobbynames[i], m_lobbyNames[i].c_str(), m_lobbyNames[i].size());
        }

        m_networkInterface->sendPacket(from, response);
    }
}