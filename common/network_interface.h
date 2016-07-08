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
        ENetPeer* m_peer;
        Packet* m_packet;
    };

    class NetworkInterface
    {
    public:
        NetworkInterface(uint16_t port);

        ~NetworkInterface();

        Packet* receivePacket(ENetPeer*& from);

        void sendPacket(ENetPeer* to, Packet* packet);

        // clear the send queue
        void writePackets();

        // TODO figure out how to to this properly
        ENetHost* m_socket;
    private:
        

        std::queue<PacketEntry> m_sendQueue;
        std::queue<PacketEntry> m_receiveQueue;
    };
}