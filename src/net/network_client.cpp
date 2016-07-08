#include <enet/enet.h>
#include "network_client.h"
#include <stdio.h>
#include <common/packet.h>
#include <common/salt.h>

namespace arena
{

    const double ConnectionRequestSendRate = 0.1;

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
        disconnect();
        m_serverAddress.port = port;
        enet_address_set_host(&m_serverAddress, address);
        m_peer = enet_host_connect(m_networkInterface.m_socket, &m_serverAddress, 2, 0);

        ENetEvent event;
        if (enet_host_service(m_networkInterface.m_socket, &event, 5000) > 0 &&
            event.type == ENET_EVENT_TYPE_CONNECT)
        {
            printf("Connection to %s,: %u succeeded", address, &port);
        }
        else
        {
            enet_peer_reset(m_peer);
            printf("Connection to %s,: %u failed", address, &port);
        }

        m_state = ClientState::SendingConnectionRequest;
        m_lastPacketSendTime = timeStamp - 1.0;
        m_lastPacketReceivedTime = timeStamp;
        m_clientSalt = genSalt();
    }

    void NetworkClient::disconnect()
    {
        if (m_state != ClientState::Disconnected)
        {
            enet_peer_disconnect(m_peer, 0);
            printf("Client side disconnect\n");
            // do something to notify server
            // sendDisconnectPacket()
        }
        reset();
    }

    bool NetworkClient::isConnected() const
    {
        return m_state == ClientState::Connected;
    }

    bool NetworkClient::isConnecting() const
    {
        return m_state == ClientState::SendingConnectionRequest;
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
            enet_address_get_host(&m_serverAddress, buffer, sizeof(buffer));
            printf("Client sending connection request to server: %s\n", buffer);
#endif
            // TODO allocator
            ConnectionRequestPacket* packet = new ConnectionRequestPacket();
            packet->m_clientSalt = m_clientSalt;

            ARENA_ASSERT(m_state != ClientState::Disconnected, "Cant send packets when disconnected");

            m_networkInterface.sendPacket(m_peer, packet);
            m_lastPacketSendTime = timestamp;
        }
            break;

        case ClientState::SendingAuthResponse:
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
    }

    void NetworkClient::receivePackets()
    {
    }

    void NetworkClient::reset()
    {
        m_state = ClientState::Disconnected;
        m_lastPacketReceivedTime = -9999.0;
        m_lastPacketSendTime = -9999.0;
    }

}