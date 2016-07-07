#include <enet/enet.h>
#include "network_client.h"
#include <stdio.h>

namespace arena
{
    NetworkClient::NetworkClient() : 
        m_state(ClientState::Disconnected), 
        m_lastPacketReceivedTime(0), 
        m_lastPacketSendTime(0)
    {

    }

    void NetworkClient::connect(const char* address, uint16_t port, double timeStamp)
    {
        disconnect();
        m_serverAddress.port = port;
        enet_address_set_host(&m_serverAddress, address);

        m_state = ClientState::SendingConnectionRequest;
        m_lastPacketSendTime = timeStamp - 1.0;
        m_lastPacketReceivedTime = timeStamp;
        // TODO timer
    }

    void NetworkClient::disconnect()
    {
        if (m_state != ClientState::Disconnected)
        {
            printf("Client side disconnect\n");
            // do something to notify server
            // sendDisconnectPacket()
        }

        m_state = ClientState::Disconnected;
        m_lastPacketReceivedTime = -9999.0;
        m_lastPacketSendTime = -9999.0;
    }

    bool NetworkClient::isConnected() const
    {
        return m_state == ClientState::Connected;
    }
}