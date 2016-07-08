#include "network_interface.h"
#include <enet\enet.h>
#include "debug.h"
#include "packet.h"

namespace arena
{
    static bool s_initialized = false;

    bool networkInitialize()
    {
        ARENA_ASSERT(s_initialized == false, "Network already initialized");

        int result = enet_initialize();

        if (result == 0)
        {
            s_initialized = true;
        }

        return s_initialized;
    }

    void networkShutdown()
    {
        ARENA_ASSERT(s_initialized == true, "Network not initialized");
        enet_deinitialize();

        s_initialized = false;
    }

    NetworkInterface::NetworkInterface(uint16_t port)
        : m_socket(nullptr)
    {
        ENetAddress address;
        address.host = ENET_HOST_ANY;
        address.port = port;
        // bound the port even to client
        m_socket = enet_host_create(&address, 1, 2, 0, 0);

        ARENA_ASSERT(m_socket != nullptr, "Failed to create socket");
    }

    NetworkInterface::~NetworkInterface()
    {
        enet_host_destroy(m_socket);
    }

    Packet* NetworkInterface::receivePacket(ENetAddress& from)
    {
        if (m_receiveQueue.size() == 0)
        {
            return nullptr;
        }

        const PacketEntry& entry = m_receiveQueue.front();
        from = entry.m_address;

        ARENA_ASSERT(entry.m_packet != nullptr, "Packet is nullptr");

        m_receiveQueue.pop();

        return entry.m_packet;
    }

    void NetworkInterface::sendPacket(const ENetAddress& to, Packet* packet)
    {
        ARENA_ASSERT(packet != nullptr, "Packet can not be nullptr");

        PacketEntry entry;
        entry.m_address = to;
        entry.m_packet = packet;

        m_sendQueue.push(entry);
    }

}