#pragma once

#include <enet/enet.h>
#include <queue>

namespace arena
{
    // start network services
    bool networkInitialize();

    // clear network services
    void networkShutdown();

    struct PacketEntry
    {
        ENetAddress m_address;
        void* m_packet;
    };

    class NetworkInterface
    {
    public:
        NetworkInterface();

        void* receivePacket(ENetAddress& from);
    private:
        std::queue<PacketEntry> m_sendQueue;
        std::queue<PacketEntry> m_receiveQueue;
    };
}