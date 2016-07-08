#pragma once

#include <enet/enet.h>
#include <queue>

namespace arena
{
    struct Packet;

    // start network services
    bool networkInitialize();

    // clear network services
    void networkShutdown();

    struct PacketEntry
    {
        ENetAddress m_address;
        Packet* m_packet;
    };

    class NetworkInterface
    {
    public:
        NetworkInterface(uint16_t port);

        ~NetworkInterface();

        Packet* receivePacket(ENetAddress& from);

        void sendPacket(const ENetAddress& to, Packet* packet);
    private:
        ENetHost* m_socket;

        std::queue<PacketEntry> m_sendQueue;
        std::queue<PacketEntry> m_receiveQueue;
    };
}