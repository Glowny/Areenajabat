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

    Packet* createPacket(int32_t type);

    struct ConnectionRequestPacket : public Packet
    {
        uint64_t m_clientSalt;

        ConnectionRequestPacket()
            : m_clientSalt(0)
        {
            
        }

        virtual ~ConnectionRequestPacket() {}

        template <typename Stream>
        bool serialize(Stream& stream)
        {
            serialize_uint64(stream, m_clientSalt);
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