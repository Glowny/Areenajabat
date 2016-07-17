#include <enet/enet.h>
#include "network_client.h"
#include <stdio.h>
#include <common/packet.h>
#include <common/salt.h>
#include <inttypes.h>
namespace arena
{

    const double ConnectionRequestSendRate = 0.1;
    const double ConnectionResponseSendRate = 0.1;
    const double ConnectionKeepAliveSendRate = 1.0;


    const double CreateLobbySendRate = 0.5;
    const double QueryLobbiesSendRate = 1.0;
    const double JoinLobbySendRate = 0.1;

    NetworkClient::NetworkClient() : 
        m_state(ClientState::Disconnected), 
        m_lastPacketReceivedTime(0), 
        m_lastPacketSendTime(0),
        m_networkInterface(nullptr),
        m_lobbyListener(nullptr)
    {
        m_socket = enet_host_create(NULL, 1, 2, 0, 0);
        m_networkInterface = new NetworkInterface(m_socket);
        reset();
    }

    void NetworkClient::connect(const char* address, uint16_t port, double timeStamp)
    {
        disconnect(timeStamp);
        m_serverAddress.port = port;
        enet_address_set_host(&m_serverAddress, address);
        m_peer = enet_host_connect(m_socket, &m_serverAddress, 2, 0);

        m_lobbyState = LobbyState::NotInLobby;
        m_state = ClientState::SendingConnectionRequest;
        m_lastPacketSendTime = timeStamp - 1.0;
        m_lastPacketReceivedTime = timeStamp;
        m_clientSalt = genSalt();

        fprintf(stderr, "Connecting to: %s:%" PRIu16 "\n", address, port);
    }

    void NetworkClient::disconnect(double timestamp)
    {
        if (m_state == ClientState::Connected)
        {
            printf("Client side disconnect\n");
            
            ConnectionDisconnectPacket* out = (ConnectionDisconnectPacket*)createPacket(PacketTypes::Disconnect);
            out->m_clientSalt = m_clientSalt;
            out->m_challengeSalt = m_challengeSalt;
            
            m_networkInterface->sendPacket(m_peer, out);
            m_lastPacketSendTime = timestamp;
        }
        reset();
    }

    bool NetworkClient::isConnected() const
    {
        return m_state == ClientState::Connected;
    }

    bool NetworkClient::isConnecting() const
    {
        return m_state == ClientState::SendingConnectionRequest || m_state == ClientState::SendingAuthResponse;
    }

    void NetworkClient::processMatchmakingPackets(Packet* packet, ENetPeer* from, double timestamp)
    {
        // not from server
        if (from->address.host != m_serverAddress.host) return;

        BX_UNUSED(from, timestamp);
        switch (packet->getType())
        {
        case PacketTypes::LobbyJoinResult:
        {
            LobbyJoinResultPacket* cast = (LobbyJoinResultPacket*)packet;

            if (cast->m_joined)
            {
                m_lobbyState = LobbyState::InLobby;
            }

            ARENA_ASSERT(m_lobbyListener != nullptr, "Lobby notifier is nullptr");

            m_lobbyListener->onLobbyJoinResult(this, cast, timestamp);
        }
        break;
        case PacketTypes::LobbyResultPacket:
        {
            m_lobbyState = LobbyState::NotInLobby;
            LobbyResultPacket* cast = (LobbyResultPacket*)packet;
            
            ARENA_ASSERT(m_lobbyListener != nullptr, "Lobby notifier is nullptr");

            m_lobbyListener->onLobbyCreationResult(this, cast, timestamp);
        }
        break;
        case PacketTypes::LobbyQueryResultPacket:
        {
            m_lobbyState = LobbyState::NotInLobby;

            LobbyQueryResultPacket* cast = (LobbyQueryResultPacket*)packet;

            ARENA_ASSERT(m_lobbyListener != nullptr, "Lobby notifier is nullptr");

            m_lobbyListener->onLobbyList(this, cast, timestamp);
        }
        break;
        }
    }

    void NetworkClient::sendMatchMakingPackets(double timestamp)
    {
        switch (m_lobbyState)
        {
        case LobbyState::SendingJoinLobby:
        {
            if (m_lastPacketSendTime + JoinLobbySendRate < timestamp)
            {
                JoinLobbyPacket* packet = (JoinLobbyPacket*)createPacket(PacketTypes::MasterJoinLobby);
                packet->m_clientSalt = m_clientSalt;
                packet->m_lobbySalt = m_currentLobby.salt;

                sendPacketToServer(packet, timestamp);
            }
        }
        break;
        case LobbyState::SendingCreateLobby:
            if (m_lastPacketSendTime + CreateLobbySendRate < timestamp)
            {
                fprintf(stderr, "Sending create lobby, name %s\n", m_currentLobby.name.c_str());
                CreateLobbyPacket* packet = (CreateLobbyPacket*)createPacket(PacketTypes::MasterCreateLobby);
                packet->m_clientSalt = m_clientSalt;
                memcpy(packet->m_name, m_currentLobby.name.c_str(), m_currentLobby.name.size());
                sendPacketToServer(packet, timestamp);
            }
            break;
        case LobbyState::SendingQueryLobbies:
            if (m_lastPacketSendTime + QueryLobbiesSendRate < timestamp)
            {
                fprintf(stderr, "Sending query lobbies\n");
                ListLobbiesPacket* packet = (ListLobbiesPacket*)createPacket(PacketTypes::MasterListLobbies);
                packet->m_clientSalt = m_clientSalt;
                sendPacketToServer(packet, timestamp);
            }
            break;
        default:
            //fprintf(stderr, "LobbyState: %d not implemented", m_lobbyState);
            break;
        }
    }

    void NetworkClient::sendProtocolPackets(double timestamp)
    {
        if (m_lobbyState != LobbyState::InLobby) return;

        switch (m_state)
        {
            // let this be the first one, so minimium amount of branching
        case arena::ClientState::Connected:
        {
            if (m_lastPacketSendTime + ConnectionKeepAliveSendRate > timestamp)
            {
                ConnectionKeepAlivePacket* packet = (ConnectionKeepAlivePacket*)createPacket(PacketTypes::KeepAlive);
                packet->m_clientSalt = m_clientSalt;
                packet->m_challengeSalt = m_challengeSalt;
                sendPacketToServer(packet, timestamp);
            }
        }
            break;
        case arena::ClientState::SendingConnectionRequest:
        {
            // have we met the send rate requirement?
            if (m_lastPacketSendTime + ConnectionRequestSendRate > timestamp)
            {
                return;
            }
#if _DEBUG
            char buffer[256];
            enet_address_get_host_ip(&m_serverAddress, buffer, sizeof(buffer));
            printf("Client sending connection request to server: %s\n", buffer);
#endif
            // TODO allocator
            ConnectionRequestPacket* packet = (ConnectionRequestPacket*)createPacket(PacketTypes::ConnectionRequest);
            packet->m_clientSalt = m_clientSalt;

            ARENA_ASSERT(m_state != ClientState::Disconnected, "Cant send packets when disconnected");

            m_networkInterface->sendPacket(m_peer, packet);
            m_lastPacketSendTime = timestamp;
        }
            break;

        case ClientState::SendingAuthResponse:
        {
            if (m_lastPacketSendTime + ConnectionResponseSendRate > timestamp) return;

            printf("Sending auth response to server\n");

            ConnectionResponsePacket* packet = (ConnectionResponsePacket*)createPacket(PacketTypes::ConnectionResponse);
            packet->m_clientSalt = m_clientSalt;
            packet->m_challengeSalt = m_challengeSalt;

            sendPacketToServer(packet, timestamp);
        }
            break;
        default:
            break;
        }
    }

    void NetworkClient::sendPacketToServer(Packet* packet, double timestamp)
    {
        ARENA_ASSERT(m_state != ClientState::Disconnected, "Can not send packets when disconnected");
        m_networkInterface->sendPacket(m_peer, packet);
        m_lastPacketSendTime = timestamp;
    }

    void NetworkClient::writePackets()
    {
        m_networkInterface->writePackets();
    }

    void NetworkClient::readPackets()
    {
        ENetEvent event;
        while (enet_host_service(m_socket, &event, 0) > 0)
        {
            if (event.type == ENET_EVENT_TYPE_CONNECT) 
            {
                printf("ENET: connected\n");
            }
            // if timeout happens
            else if (event.type == ENET_EVENT_TYPE_DISCONNECT)
            {
                printf("ENET: diconnected\n");
                m_state = ClientState::Disconnected;
                reset();
            }
            if (event.type != ENET_EVENT_TYPE_RECEIVE) continue;
            
            m_networkInterface->readPacket(event.peer, event.packet);
        }
        
    }

    void NetworkClient::requestCreateLobby(const char* name, double timestamp)
    {
        //if (m_lobbyState != LobbyState::NotInLobby) return;

        uint32_t len = (uint32_t)strlen(name);
        ARENA_ASSERT(len < CreateLobbyPacket::MaxNameLen, "Max name length exceeded");
        
        m_currentLobby.name = std::string(name);

        CreateLobbyPacket* packet = (CreateLobbyPacket*)createPacket(PacketTypes::MasterCreateLobby);
        packet->m_clientSalt = m_clientSalt;
        memcpy(packet->m_name, name, len);

        m_lobbyState = LobbyState::SendingCreateLobby;
        
        sendPacketToServer(packet, timestamp);
    }

    void NetworkClient::queryLobbies(double timestamp)
    {
        ListLobbiesPacket* packet = (ListLobbiesPacket*)createPacket(PacketTypes::MasterListLobbies);
        sendPacketToServer(packet, timestamp);

        m_lobbyState = LobbyState::SendingQueryLobbies;
    }

    void NetworkClient::requestJoinLobby(uint64_t lobbySalt, double timestamp)
    {
        m_currentLobby.salt = lobbySalt;

        JoinLobbyPacket* packet = (JoinLobbyPacket*)createPacket(PacketTypes::MasterJoinLobby);
        packet->m_clientSalt = m_clientSalt;
        packet->m_lobbySalt = lobbySalt;
        m_lobbyState = LobbyState::SendingJoinLobby;
        
        sendPacketToServer(packet, timestamp);
    }

    Packet* NetworkClient::receivePacket(ENetPeer*& from)
    {
        return m_networkInterface->receivePacket(from);
    }

    void NetworkClient::processClientSidePackets(Packet* packet, ENetPeer* peer, double timestamp)
    {
        switch (packet->getType())
        {
        case PacketTypes::KeepAlive:
        {
            // conneciton succeeded
            ConnectionKeepAlivePacket* cast = (ConnectionKeepAlivePacket*)packet;

            if (cast->m_clientSalt != m_clientSalt) break;
            if (cast->m_challengeSalt != m_challengeSalt) break;
            if (peer->address.host != m_serverAddress.host) break;

            // we are connected now
            if (m_state == ClientState::SendingAuthResponse)
            {
                char buf[256];
                enet_address_get_host_ip(&m_serverAddress, buf, sizeof(buf));
                printf("client is now connected to server %s\n", buf);
                m_state = ClientState::Connected;
            }
        }
        break;
        case PacketTypes::ConnectionChallenge:
        {
            ConnectionChallengePacket* cast = (ConnectionChallengePacket*)packet;
            if (m_state != ClientState::SendingConnectionRequest) return;
            if (cast->m_clientSalt != m_clientSalt) return;
            if (peer->address.host != m_serverAddress.host) return;

            char buf[256];
            enet_address_get_host_ip(&peer->address, buf, sizeof(buf));
            printf("received connection challenge from server: %s (salt (%" PRIx64 ")\n", buf, cast->m_challengeSalt);

            m_challengeSalt = cast->m_challengeSalt;

            m_lastPacketReceivedTime = timestamp;

            m_state = ClientState::SendingAuthResponse;
        }
        break;
        case PacketTypes::Disconnect:
        {
            ConnectionDisconnectPacket* cast = (ConnectionDisconnectPacket*)packet;
            // client side has already disconnected so this is reply from server so we shall close the socket now
            if (m_state != ClientState::Connected)
            {
                enet_peer_disconnect_later(m_peer, 0);
            }
            else if (cast->m_clientSalt != m_clientSalt) break;
            else if (cast->m_challengeSalt != m_challengeSalt) break;
            else if (peer->address.host != m_serverAddress.host) break;

            disconnect(timestamp);
        }
        break;
        default:
            fprintf(stderr, "Got invalid packet of type %d\n", packet->getType());
            break;
        }
    }

    void NetworkClient::reset()
    {
        m_lobbyState = LobbyState::NotInLobby;
        m_state = ClientState::Disconnected;
        m_challengeSalt = 0;
        m_lastPacketReceivedTime = -9999.0;
        m_lastPacketSendTime = -9999.0;
    }

}