#pragma once
#include <bx/macros.h>
#include "serialization.h"

namespace arena
{

    struct PacketTypes
    {
        enum Enum
        {
            ConnectionRequest,
            ConnectionDenied,
            ConnectionChallenge,
            ConnectionResponse,
            KeepAlive,
            Disconnect,

            Count
        };
    };

    struct BX_NO_VTABLE Packet
    {
        virtual ~Packet() = 0;
        virtual uint32_t getType() const = 0;
    };

    inline Packet::~Packet() {}

    struct ConnectionRequestPacket : public Packet
    {
        uint64_t m_clientSalt;
        uint8_t m_data[256];

        ConnectionRequestPacket()
            : m_clientSalt(0)
        {
            memset(m_data, 0, sizeof(m_data));
        }

        virtual ~ConnectionRequestPacket() {}

        template <typename Stream>
        bool serialize(Stream& stream)
        {
            serialize_uint64(stream, m_clientSalt);
            if (Stream::IsReading && stream.getBitsRemaining() < 256 * 8)
            {
                return false;
            }
            serialize_bytes(stream, data, sizeof(m_data));
            return true;
        }

        virtual uint32_t getType() const override
        {
            return PacketTypes::ConnectionRequest;
        }
    };
}