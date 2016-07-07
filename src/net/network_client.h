#pragma once

#include <stdint.h>
#include <enet/enet.h>

namespace arena
{
    struct ClientState
    {
        enum Enum
        {
            Disconnected,
            Connected,
            SendingConnectionRequest
        };
    };

    class NetworkClient
    {
    public:
        NetworkClient();

        void connect(const char* address, uint16_t port, double timeStamp);

        void disconnect();

        bool isConnected() const;

    private:
        ClientState::Enum m_state;

        ENetAddress m_serverAddress;
        // time we last sent a packet
        double m_lastPacketSendTime;

        // time we last received a packet
        double m_lastPacketReceivedTime;
    };
}