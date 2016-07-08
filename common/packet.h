#pragma once
#include <bx/macros.h>
#include "serialization.h"
#include "read_stream.h"
#include "write_stream.h"
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
        virtual int32_t getType() const = 0;
        virtual bool serializeWrite(WriteStream& stream) = 0;
        virtual bool serializeRead(ReadStream& stream) = 0;
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
            serialize_bytes(stream, m_data, sizeof(m_data));
            return true;
        }

        virtual int32_t getType() const override
        {
            return PacketTypes::ConnectionRequest;
        }

        bool serializeWrite(WriteStream& stream) override
        {
            return serialize(stream);
        }

        bool serializeRead(ReadStream& stream) override
        {
            return serialize(stream);
        }
    };
}