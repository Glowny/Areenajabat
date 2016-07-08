#include <enet/enet.h>
#include "network_client.h"
#include <stdio.h>
#include <common/packet.h>
#include <common/salt.h>

namespace arena
{

    const double ConnectionRequestSendRate = 0.1;

    NetworkClient::NetworkClient() : 
        m_state(ClientState::Disconnected), 
        m_lastPacketReceivedTime(0), 
        m_lastPacketSendTime(0)
    {
        reset();
    }

    void NetworkClient::connect(const char* address, uint16_t port, double timeStamp)
    {
        disconnect();
        m_serverAddress.port = port;
        enet_address_set_host(&m_serverAddress, address);

        m_state = ClientState::SendingConnectionRequest;
        m_lastPacketSendTime = timeStamp - 1.0;
        m_lastPacketReceivedTime = timeStamp;
        m_clientSalt = genSalt();
    }

    void NetworkClient::disconnect()
    {
        if (m_state != ClientState::Disconnected)
        {
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

            

            m_lastPacketSendTime = timestamp;
        }
            break;

        case ClientState::SendingAuthResponse:
            break;
        default:
            break;
        }
    }

    void NetworkClient::reset()
    {
        m_state = ClientState::Disconnected;
        m_lastPacketReceivedTime = -9999.0;
        m_lastPacketSendTime = -9999.0;
    }

}