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

    NetworkInterface::NetworkInterface(uint16_t port)
        : m_socket(nullptr)
    {
        ENetAddress address;
        address.host = ENET_HOST_ANY;
        address.port = port;
        // bound the port even to client
        m_socket = enet_host_create(&address, 32, 2, 0, 0);

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

    void NetworkInterface::readPackets()
    {
        // todo hax
        const uint32_t MaxPacketSize = 512;
        uint8_t packetBuffer[MaxPacketSize];
        (void)packetBuffer;

        while (true)
        {
            ENetPeer* peer; (void)peer;
            ENetEvent event;
            while (enet_host_service(m_socket, &event, 0) > 0)
            {
#if _DEBUG
                if (event.type == ENET_EVENT_TYPE_CONNECT) printf("ENET: connected\n");
                else if (event.type == ENET_EVENT_TYPE_DISCONNECT) printf("ENET: diconnected\n");
#endif
                if (event.type != ENET_EVENT_TYPE_RECEIVE) continue;

                uint32_t packetBytes = uint32_t(event.packet->dataLength);
                ReadStream stream(event.packet->data, packetBytes);
                
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
                    enet_packet_destroy(event.packet);
                    continue;
                }

                PacketEntry entry;
                entry.m_packet = packet;
                entry.m_peer = event.peer;

                m_receiveQueue.push(entry);

                enet_packet_destroy(event.packet);

#if _DEBUG
                char buffer[256];
                enet_address_get_host_ip(&event.peer->address, buffer, sizeof(buffer));
                printf("Got packet of type %d from %s\n", packetType, buffer);
#endif
            }
            break;
        }
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

            ENetPacket* out = enet_packet_create(packetbuffer, packetLength, ENET_PACKET_FLAG_NO_ALLOCATE);
            
            enet_peer_send(entry.m_peer, 0, out);

            enet_host_flush(m_socket);

            //enet_packet_destroy(out);

            delete packet;
        }
    }

}