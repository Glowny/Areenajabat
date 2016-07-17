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
        NetworkInterface(ENetHost* socket);

        ~NetworkInterface();

        Packet* receivePacket(ENetPeer*& from);

        // Call this before receivePacket
        // Serializes the incoming ENet packet and enqueues it to receivequeue
        // Frees enet packet
        void readPacket(ENetPeer* from, ENetPacket* packet);

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