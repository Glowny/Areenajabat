#include "network_interface.h"
#include <enet\enet.h>
#include "debug.h"
#include "packet.h"
#include "write_stream.h"

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

    NetworkInterface::NetworkInterface(ENetHost* socket)
        : m_socket(socket)
    {
        ARENA_ASSERT(m_socket != nullptr, "Failed to create socket");
    }

    NetworkInterface::~NetworkInterface()
    {
        enet_host_destroy(m_socket);
    }

    Packet* NetworkInterface::receivePacket(ENetPeer*& from)
    {
        if (m_receiveQueue.size() == 0)
        {
            return nullptr;
        }

        const PacketEntry& entry = m_receiveQueue.front();
        from = entry.m_peer;

        ARENA_ASSERT(entry.m_packet != nullptr, "Packet is nullptr");

        m_receiveQueue.pop();

        return entry.m_packet;
    }

    void NetworkInterface::readPacket(ENetPeer* from, ENetPacket* enetPacket)
    {
        uint32_t packetBytes = uint32_t(enetPacket->dataLength);
        ReadStream stream(enetPacket->data, packetBytes);

        /*
        uint32_t prefixBytes = 1;
        for (uint32_t i = 0; i < prefixBytes; ++i)
        {
            uint32_t dummy = 0;
            stream.serializeBits(dummy, 8);
        }*/

        uint32_t crc32 = 0;

        stream.serializeBits(crc32, 32);

        int32_t packetType = 0;
        if (!stream.serializeInteger(packetType, 0, PacketTypes::Count - 1))
        {
            ARENA_ASSERT(0, "invalid packet");
        }

        Packet* packet = createPacket(packetType);

        ARENA_ASSERT(packet != nullptr, "Packet is nullptr");

        if (!packet->serializeRead(stream))
        {
            fprintf(stderr, "Failed to serialize packet of type %d", packetType);
            enet_packet_destroy(enetPacket);
            return;
        }

        PacketEntry entry;
        entry.m_packet = packet;
        entry.m_peer = from;

        ARENA_ASSERT(stream.m_error.isOk(), "serialization error: %d", stream.m_error.get().code);

        m_receiveQueue.push(entry);

        enet_packet_destroy(enetPacket);
    }

    void NetworkInterface::sendPacket(ENetPeer* to, Packet* packet)
    {
        ARENA_ASSERT(packet != nullptr, "Packet can not be nullptr");

        PacketEntry entry;
        entry.m_peer = to;
        entry.m_packet = packet;

        m_sendQueue.push(entry);
    }

    void NetworkInterface::writePackets()
    {
        while (!m_sendQueue.empty())
        {
            PacketEntry& entry = m_sendQueue.front();

            ARENA_ASSERT(entry.m_packet != nullptr, "Packet is nullptr");

            m_sendQueue.pop();

            uint8_t packetbuffer[512];
            WriteStream stream(packetbuffer, sizeof(packetbuffer));

            /*uint32_t prefixBytes = 1;
            for (uint32_t i = 0; i < prefixBytes; ++i)
            {
                uint8_t zero = 0;
                stream.serializeBits(zero, 8);
            }*/

            uint32_t crc32 = 0;

            stream.serializeBits(crc32, 32);

            Packet* packet = entry.m_packet;

            int packetType = packet->getType();

            stream.serializeInteger(packetType, 0, PacketTypes::Count - 1);

            if (!packet->serializeWrite(stream))
            {
                ARENA_ASSERT(0, "shit hit the fan");
            }

            stream.flush();

            uint32_t packetLength = stream.getBytesProcessed();

            ENetPacket* out = enet_packet_create(packetbuffer, packetLength, 0);
            
            enet_peer_send(entry.m_peer, 0, out);

            ARENA_ASSERT(stream.m_error.isOk(), "serialization error: %d", stream.m_error.get().code);

            // broadcast if necessary, because all the packets should be queued next to each other
            while (!m_sendQueue.empty())
            {
                PacketEntry& another = m_sendQueue.front();
                // if it's same packet, we dont need to serialize it again
                // send it to all peers
                if (another.m_packet != entry.m_packet) break;

                enet_peer_send(another.m_peer, 0, out);
                another.m_packet = nullptr;
                
                m_sendQueue.pop();
            }
 
            entry.m_packet = nullptr;

            destroyPacket(packet);
        }
    }

}