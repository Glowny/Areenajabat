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

    NetworkClient::NetworkClient(uint16_t clientPort) : 
        m_state(ClientState::Disconnected), 
        m_lastPacketReceivedTime(0), 
        m_lastPacketSendTime(0),
        m_networkInterface(clientPort)
    {
        reset();
    }

    void NetworkClient::connect(const char* address, uint16_t port, double timeStamp)
    {
        disconnect(timeStamp);
        m_serverAddress.port = port;
        enet_address_set_host(&m_serverAddress, address);
        m_peer = enet_host_connect(m_networkInterface.m_socket, &m_serverAddress, 2, 0);

        m_state = ClientState::SendingConnectionRequest;
        m_lastPacketSendTime = timeStamp - 1.0;
        m_lastPacketReceivedTime = timeStamp;
        m_clientSalt = genSalt();
    }

    void NetworkClient::disconnect(double timestamp)
    {
        if (m_state == ClientState::Connected)
        {
            printf("Client side disconnect\n");
            
            ConnectionDisconnectPacket* out = (ConnectionDisconnectPacket*)createPacket(PacketTypes::Disconnect);
            out->m_clientSalt = m_clientSalt;
            out->m_challengeSalt = m_challengeSalt;
            
            m_networkInterface.sendPacket(m_peer, out);
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

    void NetworkClient::sendPackets(double timestamp)
    {
        switch (m_state)
        {
            // let this be the first one, so minimium amount of branching
        case arena::ClientState::Connected:
            // for pings
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

            m_networkInterface.sendPacket(m_peer, packet);
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

            ARENA_ASSERT(m_state != ClientState::Disconnected, "Cant send packets when disconnected");

            m_networkInterface.sendPacket(m_peer, packet);
            m_lastPacketSendTime = timestamp;
        }
            break;
        default:
            break;
        }
    }

    void NetworkClient::writePackets()
    {
        m_networkInterface.writePackets();
    }

    void NetworkClient::readPackets()
    {
        m_networkInterface.readPackets();
    }

    void NetworkClient::receivePackets(double timestamp)
    {
        while (true)
        {
            ENetPeer* peer;
            Packet* packet = m_networkInterface.receivePacket(peer);

            if (packet == nullptr)
            {
                break;
            }

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
                    enet_peer_disconnect(m_peer, 0);
                }
                else if (cast->m_clientSalt != m_clientSalt) break;
                else if (cast->m_challengeSalt != m_challengeSalt) break;
                else if (peer->address.host != m_serverAddress.host) break;

                disconnect(timestamp);                
            }
            break;
            default:
                fprintf(stderr, "Got invalid packet of type %d (not implemented?)\n", packet->getType());
                break;
            }

            destroyPacket(packet);
        }
    }

    void NetworkClient::reset()
    {
        m_state = ClientState::Disconnected;
        m_challengeSalt = 0;
        m_lastPacketReceivedTime = -9999.0;
        m_lastPacketSendTime = -9999.0;
    }

}