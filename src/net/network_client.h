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
            SendingAuthResponse,
            SendingConnectionRequest,
            Count,
        };
    };

    class NetworkClient
    {
    public:
        NetworkClient();

        void connect(const char* address, uint16_t port, double timestamp);

        void disconnect();

        bool isConnected() const;

        bool isConnecting() const;

        void sendPackets(double timestamp);
    private:

        void reset();

        ClientState::Enum m_state;

        ENetAddress m_serverAddress;
        // time we last sent a packet
        double m_lastPacketSendTime;

        // time we last received a packet
        double m_lastPacketReceivedTime;

        // client salt for challenge auth
        uint64_t m_clientSalt;
    };
}