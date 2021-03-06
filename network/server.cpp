#include "server.h"
#include <bx/bx.h>
#include <stdio.h>
#include <assert.h>
#include <bx/timer.h>
#include <stdint.h>
#include <common/salt.h>
#include <minini/minIni.h>
#include "common/packet.h"
#include <stdio.h>
#include "game_host.h"
#include "client_listener.h"

namespace arena
{
    

    static const double ChallengeTimeOut = 10.0;
    static const double ChallengeSendRate = 0.1;
    static const double ConnectionConfirmSendRate = 0.1;

    static const double PingTimeOut = 10.0;

    Server::Server(std::vector<PacketEntry>* sendQueue) :
        m_clientsConnected(0),
        m_sendQueue(sendQueue)
    {
        memset(m_clientPeers, 0, sizeof(m_clientPeers));
        memset(m_clientConnected, 0, sizeof(m_clientConnected));
        memset(m_clientSalt, 0, sizeof(m_clientConnected));
        memset(m_challengeSalt, 0, sizeof(m_challengeSalt));

        m_serverSalt = arena::genSalt();

        for (uint32_t i = 0; i < MaxClients; ++i)
        {
            m_clientIndices[i] = i;
        }
    }

    Server::~Server()
    {

    }

    void Server::addClientListener(ClientListener* listener)
    {
        m_listeners.push_back(listener);
    }

    void Server::processPacket(Packet* packet, ENetPeer* peer, double timestamp)
    {
        switch (packet->getType())
        {
        case PacketTypes::ConnectionRequest:
            processConnectionRequest((ConnectionRequestPacket*)packet, peer, timestamp);
            break;
        case PacketTypes::ConnectionResponse:
            processConnectionResponse((ConnectionResponsePacket*)packet, peer, timestamp);
            break;
        case PacketTypes::KeepAlive:
            processConnectionKeepAlive((ConnectionKeepAlivePacket*)packet, peer, timestamp);
            break;
        case PacketTypes::Disconnect:
            processConnectionDisconnect((ConnectionDisconnectPacket*)packet, peer, timestamp);
            break;
        default:
            fprintf(stderr, "Got invalid packet of type %d (not implemented?)", packet->getType());
            break;
        }
    }

    void Server::connectClient(uint32_t clientIndex, ENetPeer* peer, uint64_t clientSalt, uint64_t challengeSalt, double connectTime)
    {
        ARENA_ASSERT(m_clientsConnected < MaxClients - 1, "Server is full");
        ARENA_ASSERT(!m_clientConnected[clientIndex], "Client state is already set to connected");

        ++m_clientsConnected;

        m_clientConnected[clientIndex] = true;

        m_clientData[clientIndex].m_peer = peer;
        m_clientData[clientIndex].m_challengeSalt = challengeSalt;
        m_clientData[clientIndex].m_clientSalt = clientSalt;
        m_clientData[clientIndex].m_connectTime = connectTime;
        m_clientData[clientIndex].m_lastPacketReceiveTime = connectTime;
        m_clientData[clientIndex].m_lastPacketSendTime = connectTime;
        
        m_challengeSalt[clientIndex] = challengeSalt;
        m_clientPeers[clientIndex] = peer;
        m_clientSalt[clientIndex] = clientSalt;

        char buffer[256];
        enet_address_get_host_ip(&peer->address, buffer, sizeof(buffer));
        printf("client %d connected (client address = %s, client salt = %" PRIx64 ", challenge salt = %" PRIx64 ")\n", clientIndex, buffer, clientSalt, challengeSalt);

        ConnectionKeepAlivePacket* ping = (ConnectionKeepAlivePacket*)createPacket(PacketTypes::KeepAlive);
        ping->m_clientSalt = m_clientSalt[clientIndex];
        ping->m_challengeSalt = m_challengeSalt[clientIndex];

        sendPacketToConnectedClient(clientIndex, ping, connectTime);
    }

    void Server::disconnectClient(uint32_t clientIndex, double timestamp)
    {
        ARENA_ASSERT(clientIndex < MaxClients, "clientIndex out of bounds");
        ARENA_ASSERT(m_clientsConnected > 0, "No clients connected");
        ARENA_ASSERT(m_clientConnected[clientIndex], "Client index %d ins't connected", clientIndex);

        ConnectionDisconnectPacket* packet = (ConnectionDisconnectPacket*)createPacket(PacketTypes::Disconnect);
        packet->m_clientSalt = m_clientData[clientIndex].m_clientSalt;
        packet->m_challengeSalt = m_clientData[clientIndex].m_challengeSalt;

        sendPacketToConnectedClient(clientIndex, packet, timestamp);

        // notify listeners before destroying data if they need something to lookup

        for (ClientListener* listener : m_listeners)
        {
            listener->onClientDisconnected(clientIndex, m_clientPeers[clientIndex], timestamp);
        }

        resetClient(clientIndex);
        --m_clientsConnected;
    }

    void Server::sendPacketToConnectedClient(uint32_t clientIndex, Packet* packet, double timestamp)
    {
        ARENA_ASSERT(clientIndex < MaxClients, "Invalid client index");
        ARENA_ASSERT(m_clientConnected[clientIndex], "Client isn't connected");
        ARENA_ASSERT(packet != nullptr, "Packet can not be NULL");
        ARENA_ASSERT(m_clientPeers[clientIndex] != nullptr, "Peer can not be NULL");

        m_sendQueue->push_back(PacketEntry{ m_clientPeers[clientIndex], packet });
        m_clientData[clientIndex].m_lastPacketSendTime = timestamp;
    }

    uint32_t Server::findExistingClientIndex(ENetPeer* host, uint64_t clientSalt, uint64_t challengeSalt) const
    {
        enet_uint32 address = host->address.host;
        for (uint32_t i = 0; i < MaxClients; ++i)
        {
            if (m_clientConnected[i] &&
                m_clientPeers[i]->address.host == address
                && m_clientSalt[i] == clientSalt
                && m_challengeSalt[i] == challengeSalt)
            {
                return i;
            }
        }
        return UINT32_MAX;
    }

    uint32_t Server::findFreeClientIndex()
    {
        for (uint32_t i = 0; i < MaxClients; ++i)
        {
            if (!m_clientConnected[i]) return i;
        }

        return UINT32_MAX;
    }

    void Server::processConnectionRequest(ConnectionRequestPacket* packet, ENetPeer* from, double timestamp)
    {
        BX_UNUSED(packet, timestamp);
        char buffer[256];
        enet_address_get_host_ip(&from->address, buffer, sizeof(buffer));
        printf("GOt connection request packet from: %s\n", buffer);

		// TODO: read from game vars.
        if (m_clientsConnected == MaxClients)
        {
            printf("Server is full\n");
            ConnectionDeniedPacket* denied = (ConnectionDeniedPacket*)createPacket(PacketTypes::ConnectionDenied);
            denied->m_clientSalt = packet->m_clientSalt;
            denied->m_reason = ConnectionDeniedPacket::ConnectionDeniedReason::ServerIsFull;
            
            m_sendQueue->push_back(PacketEntry{ from, denied });
        }

        else if (isConnected(packet->m_clientSalt, from))
        {
            printf("Client (%" PRIu64 ") is already connected!\n", packet->m_clientSalt);

            ConnectionDeniedPacket* denied = (ConnectionDeniedPacket*)createPacket(PacketTypes::ConnectionDenied);
            denied->m_clientSalt = packet->m_clientSalt;
            denied->m_reason = ConnectionDeniedPacket::ConnectionDeniedReason::AlreadyConnected;

            m_sendQueue->push_back(PacketEntry{ from, denied });
        }

        ServerChallengeEntry* entry = findOrInsertChallenge(from, packet->m_clientSalt, timestamp);
        
        if (entry == nullptr)
        {
            return;
        }

        ARENA_ASSERT(entry->m_peer->address.host == from->address.host, "Host mismatch");
        ARENA_ASSERT(entry->m_clientSalt == packet->m_clientSalt, "Client salt mismatch");

        if (entry->m_lastSendTime + ChallengeSendRate < timestamp)
        {
            printf("Sending connection challenge to %s (challenge salt = %" PRIx64 ")\n", buffer, entry->m_challengeSalt);
            ConnectionChallengePacket* packet = (ConnectionChallengePacket*)createPacket(PacketTypes::ConnectionChallenge);
            packet->m_clientSalt = entry->m_clientSalt;
            packet->m_challengeSalt = entry->m_challengeSalt;
            
            m_sendQueue->push_back(PacketEntry{ from, packet });

            entry->m_lastSendTime = timestamp;
        }
    }

    void Server::processConnectionResponse(ConnectionResponsePacket* packet, ENetPeer* from, double timestamp)
    {
        uint32_t idx = findExistingClientIndex(from, packet->m_clientSalt, packet->m_challengeSalt);

        // if it exists
        if (idx != UINT32_MAX)
        {
            ARENA_ASSERT(idx < MaxClients, "Invalid client id (%" PRIu32 ") max (%" PRIu32 ")", idx, MaxClients);

            if (m_clientData[idx].m_lastPacketSendTime + ConnectionConfirmSendRate < timestamp)
            {
                ConnectionKeepAlivePacket* ping = (ConnectionKeepAlivePacket*)createPacket(PacketTypes::KeepAlive);
                ping->m_clientSalt = packet->m_clientSalt;
                ping->m_challengeSalt = packet->m_challengeSalt;
                sendPacketToConnectedClient(idx, ping, timestamp);
            }
        }
        else
        {
            char buf[256];
            enet_address_get_host_ip(&from->address, buf, sizeof(buf));
            printf("processing connection response from client %s (salt %" PRIx64 ") (challenge %" PRIx64 ")\n", buf, packet->m_clientSalt, packet->m_challengeSalt);

            ServerChallengeEntry* entry = findChallenge(from, packet->m_clientSalt);

            if (entry == nullptr) return;

            ARENA_ASSERT(entry->m_clientSalt == packet->m_clientSalt, "Client salt mismatch");

            if (entry->m_challengeSalt != packet->m_challengeSalt)
            {
                printf("conneciton challenge mismatch expected %" PRIx64 ", got %" PRIx64 "\n", entry->m_challengeSalt, packet->m_challengeSalt);
                return;
            }

            if (m_clientsConnected == MaxClients)
            {
                // TODO send denied
                ARENA_ASSERT(0, "serverf is full");
            }

            const uint32_t clientIndex = findFreeClientIndex();

            ARENA_ASSERT(clientIndex != UINT32_MAX, "Invalid client index");

            connectClient(clientIndex, from, packet->m_clientSalt, packet->m_challengeSalt, timestamp);

            for (uint32_t i = 0; i < (uint32_t)m_listeners.size(); ++i)
            {
                m_listeners[i]->onClientConnected(clientIndex, from, timestamp);
            }
        }
    }

    void Server::processConnectionKeepAlive(ConnectionKeepAlivePacket* packet, ENetPeer* from, double time)
    {
        const uint32_t idx = findExistingClientIndex(from, packet->m_clientSalt, packet->m_challengeSalt);
        // does not exist
        if (idx == UINT32_MAX) return;

        ARENA_ASSERT(idx < MaxClients, "Client id out of bounds");

        m_clientData[idx].m_lastPacketReceiveTime = time;
    }

    void Server::processConnectionDisconnect(ConnectionDisconnectPacket* packet, ENetPeer* from, double timestamp)
    {
#if _DEBUG
        char ip[256]; enet_address_get_host_ip(&from->address, ip, sizeof(ip));
        fprintf(stderr, "Got disconnect request from %s ", ip);
#endif

        const uint32_t idx = findExistingClientIndex(from, packet->m_clientSalt, packet->m_challengeSalt);

        if (idx == UINT32_MAX) return;

        ARENA_ASSERT(idx < MaxClients, "Client id out of bounds");

#if _DEBUG
        fprintf(stderr, "disconnecting client %" PRIu32 "\n", idx);
#endif

        disconnectClient(idx, timestamp);
    }

    ServerChallengeEntry* Server::findOrInsertChallenge(ENetPeer* from, uint64_t clientSalt, double timestamp)
    {
        const uint64_t key = calculateChallengeHash(from, clientSalt, m_serverSalt);

        uint32_t index = key % ChallengeHashSize;

#if 0
        printf("Client salt    = %" PRIx64 "\n", clientSalt);
        printf("Challenge hash = %" PRIx64 "\n", key);
        printf("Challenge idx  = %" PRIu32 "\n", index);
#endif

        if (!m_challengeHash.m_exists[index] || (m_challengeHash.m_exists[index] && m_challengeHash.m_entries[index].m_createdTime + ChallengeTimeOut < timestamp))
        {
#if 0
            printf("found empty entry in challenge hash (idx = %" PRIu32 ")\n", index);
#endif
            ServerChallengeEntry* entry = &m_challengeHash.m_entries[index];
            entry->m_clientSalt = clientSalt;
            entry->m_challengeSalt = genSalt();
            // force send
            entry->m_createdTime = timestamp;
            entry->m_lastSendTime = timestamp - (2.0 * ChallengeSendRate); // hmm
            entry->m_peer = from;

            m_challengeHash.m_exists[index] = 1;

            return entry;
        }

        if (m_challengeHash.m_exists[index]
            && m_challengeHash.m_entries[index].m_clientSalt == clientSalt
            && m_challengeHash.m_entries[index].m_peer->address.host == from->address.host)
        {
#if 0
            printf("found existing challenge hash (idx = %" PRIu32 ")\n", index);
#endif
            return &m_challengeHash.m_entries[index];
        }
        return nullptr;
    }

    ServerChallengeEntry* Server::findChallenge(ENetPeer* from, uint64_t clientSalt)
    {
        const uint64_t key = calculateChallengeHash(from, clientSalt, m_serverSalt);

        uint32_t index = key % ChallengeHashSize;

#if 0
        printf("Client salt    = %" PRIx64 "\n", clientSalt);
        printf("Challenge hash = %" PRIx64 "\n", key);
        printf("Challenge idx  = %" PRIu32 "\n", index);
#endif

        if (m_challengeHash.m_exists[index]
            && m_challengeHash.m_entries[index].m_clientSalt == clientSalt
            && m_challengeHash.m_entries[index].m_peer->address.host == from->address.host)
        {
#if 0
            printf("found existing challenge hash (idx = %" PRIu32 ")\n", index);
#endif
            return &m_challengeHash.m_entries[index];
        }
        return nullptr;
    }

    void Server::resetClient(uint32_t clientIndex)
    {
        ARENA_ASSERT(clientIndex < MaxClients, "Client index out of bounds");
        m_clientConnected[clientIndex] = false;
        m_clientSalt[clientIndex] = 0;
        m_challengeSalt[clientIndex] = 0;
        m_clientPeers[clientIndex] = 0;
        m_clientData[clientIndex] = ClientData();
    }

    bool Server::isConnected(uint64_t clientSalt, ENetPeer* peer)
    {
        ARENA_ASSERT(peer != nullptr, "Peer can not be nullptr");
        ARENA_ASSERT(clientSalt != 0, "Invalid client salt");

        for (uint32_t i = 0; i < MaxClients; ++i)
        {
            if (!m_clientConnected[i]) continue;

            if (m_clientSalt[i] == clientSalt && m_clientPeers[i]->address.host == peer->address.host)
                return true;
        }
        return false;
    }

    void Server::checkTimeout(double timestamp)
    {
        for (uint32_t i = 0; i < MaxClients; ++i)
        {
            if (!m_clientConnected[i]) continue;

            if (m_clientData[i].m_lastPacketReceiveTime + PingTimeOut < timestamp)
            {
                char buffer[256];
                enet_address_get_host_ip(&m_clientPeers[i]->address, buffer, sizeof(buffer));
                fprintf(stderr, "Client (idx = %" PRIu32 ") (address = %s) timed out, closing link\n", i, buffer);

                // disconnect nullifies this
                ENetPeer* clientPeer = m_clientPeers[i];

                disconnectClient(i, timestamp);
                enet_peer_disconnect_later(clientPeer, 0);
            }
        }
    }

}