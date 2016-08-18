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
			// Connection upkeep packets.
            ConnectionRequest,
            ConnectionDenied,
            ConnectionChallenge,
            ConnectionResponse,
            KeepAlive,
            Disconnect, // Keep "Disconnect" as last connection upkeep packet.
			// Connection upkeep end.

			// Lobby finding packets.
            // Client --> server
            MasterCreateLobby,
            MasterJoinLobby,
            MasterListLobbies,

            // Server --> Client
            LobbyResultPacket,
            LobbyQueryResultPacket,
            LobbyJoinResult, // Keep "LobbyJoinResult" as last lobby finding-related packet.
			// Lobby finding end.

			// Game related packets.
			// Server --> Client.
			GameSetup,
			GameUpdate,
			GameCreateGladiators,
			GamePlatform,
			GameSpawnBullets,
			GameBulletHit,
			GameDamagePlayer,
			GameKillPlayer,
			GameRespawnPlayer,
			GameUpdateScoreBoard,
			GameMode,

			// Client --> Server.
			GameInput,
			GameShoot,
			GameRequestMap,

			// Game debug
			// Server --> Client.
			GameBulletCurrentPosition,

			// Client --> Server.
			GameSetPlayerAmount,

            Count
        };
    };

    struct BX_NO_VTABLE Packet
    {
		Packet() = default;
		virtual ~Packet();

        virtual int32_t getType() const = 0;
        virtual bool serializeWrite(WriteStream& stream) = 0;
        virtual bool serializeRead(ReadStream& stream) = 0;
    };

    inline Packet::~Packet() {}

	Packet* createPacket(int32_t type);

    void destroyPacket(Packet* packet);

    size_t getMaxPacketSize(int32_t type);

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

    struct ConnectionDeniedPacket : public Packet
    {
        struct ConnectionDeniedReason
        {
            enum Enum
            {
                ServerIsFull,
                AlreadyConnected,

                Count
            };
        };

        uint64_t m_clientSalt;
        ConnectionDeniedReason::Enum m_reason;

        ConnectionDeniedPacket()
            : m_clientSalt(0)
        {

        }

        virtual ~ConnectionDeniedPacket() {}

        template <typename Stream>
        bool serialize(Stream& stream)
        {
            serialize_uint64(stream, m_clientSalt);
            return true;
        }

        virtual int32_t getType() const override
        {
            return PacketTypes::ConnectionDenied;
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

    struct ConnectionChallengePacket : public Packet
    {
        uint64_t m_clientSalt;
        uint64_t m_challengeSalt;

        ConnectionChallengePacket() :
            m_clientSalt(0),
            m_challengeSalt(0)
        {

        }

        virtual ~ConnectionChallengePacket() {}

        template <typename Stream>
        bool serialize(Stream& stream)
        {
            serialize_uint64(stream, m_clientSalt);
            serialize_uint64(stream, m_challengeSalt);
            return true;
        }

        virtual int32_t getType() const override
        {
            return PacketTypes::ConnectionChallenge;
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

    struct ConnectionResponsePacket : public Packet
    {
        uint64_t m_clientSalt; // the sender id
        uint64_t m_challengeSalt; // got from server and sent back to server to confirm that auth happened

        ConnectionResponsePacket() :
            m_clientSalt(0),
            m_challengeSalt(0)
        {

        }

        virtual ~ConnectionResponsePacket() {}

        template <typename Stream>
        bool serialize(Stream& stream)
        {
            serialize_uint64(stream, m_clientSalt);
            serialize_uint64(stream, m_challengeSalt);
            return true;
        }

        virtual int32_t getType() const override
        {
            return PacketTypes::ConnectionResponse;
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

    struct ConnectionKeepAlivePacket : public Packet
    {
        uint64_t m_clientSalt; // the sender id
        uint64_t m_challengeSalt; // got from server and sent back to server to confirm that auth happened

        ConnectionKeepAlivePacket() :
            m_clientSalt(0),
            m_challengeSalt(0)
        {

        }

        virtual ~ConnectionKeepAlivePacket() {}

        template <typename Stream>
        bool serialize(Stream& stream)
        {
            serialize_uint64(stream, m_clientSalt);
            serialize_uint64(stream, m_challengeSalt);
            return true;
        }

        virtual int32_t getType() const override
        {
            return PacketTypes::KeepAlive;
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

    struct ConnectionDisconnectPacket : public Packet
    {
        uint64_t m_clientSalt; // the sender id
        uint64_t m_challengeSalt; // got from server and sent back to server to confirm that auth happened

        ConnectionDisconnectPacket() :
            m_clientSalt(0),
            m_challengeSalt(0)
        {

        }

        virtual ~ConnectionDisconnectPacket() {}

        template <typename Stream>
        bool serialize(Stream& stream)
        {
            serialize_uint64(stream, m_clientSalt);
            serialize_uint64(stream, m_challengeSalt);
            return true;
        }

        virtual int32_t getType() const override
        {
            return PacketTypes::Disconnect;
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

    struct CreateLobbyPacket : public Packet
    {
        static const uint32_t MaxNameLen = 64;

        uint64_t m_clientSalt; // the sender id
        char m_name[64];

        CreateLobbyPacket() :
            m_clientSalt(0)
        {
            memset(m_name, 0, sizeof(m_name));
        }

        virtual ~CreateLobbyPacket() {}

        template <typename Stream>
        bool serialize(Stream& stream)
        {
            serialize_uint64(stream, m_clientSalt);
            serialize_string(stream, m_name, MaxNameLen);
            return true;
        }

        virtual int32_t getType() const override
        {
            return PacketTypes::MasterCreateLobby;
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

    struct JoinLobbyPacket : public Packet
    {
        uint64_t m_clientSalt; // the sender id
        uint64_t m_lobbySalt; // which lobby to join

        JoinLobbyPacket() :
            m_clientSalt(0),
            m_lobbySalt(0)
        {

        }

        virtual ~JoinLobbyPacket() {}

        template <typename Stream>
        bool serialize(Stream& stream)
        {
            serialize_uint64(stream, m_clientSalt);
            serialize_uint64(stream, m_lobbySalt);
            return true;
        }

        virtual int32_t getType() const override
        {
            return PacketTypes::MasterJoinLobby;
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

    struct ListLobbiesPacket : public Packet
    {
        uint64_t m_clientSalt; // the sender id

        ListLobbiesPacket() :
            m_clientSalt(0)
        {

        }

        virtual ~ListLobbiesPacket() {}

        template <typename Stream>
        bool serialize(Stream& stream)
        {
            serialize_uint64(stream, m_clientSalt);
            return true;
        }

        virtual int32_t getType() const override
        {
            return PacketTypes::MasterListLobbies;
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

    struct LobbyResultPacket : public Packet
    {
        uint64_t m_clientSalt; // the sender id
        int32_t m_created;
        uint64_t m_lobbySalt;

        LobbyResultPacket() :
            m_clientSalt(0),
            m_lobbySalt(0),
            m_created(false)
        {
            //memset(m_reason, 0, sizeof(m_reason));
        }

        virtual ~LobbyResultPacket() {}

        template <typename Stream>
        bool serialize(Stream& stream)
        {
            serialize_uint64(stream, m_clientSalt);
            serialize_int(stream, m_created, 0, 1);

            if (m_created)
            {
                serialize_uint64(stream, m_lobbySalt);
            }
            else
            {
                if (Stream::IsReading)
                {
                    m_lobbySalt = 0;
                }
            }
            return true;
        }

        virtual int32_t getType() const override
        {
            return PacketTypes::LobbyResultPacket;
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

    struct LobbyJoinResultPacket : public Packet
    {
        enum Reason
        {
            LobbyDoesNotExist,
            AlreadyJoined,
            Count
        };

        uint64_t m_clientSalt; // the sender id
        int32_t m_joined;
        int32_t m_reason;

        LobbyJoinResultPacket() :
            m_joined(false),
            m_reason(Reason::Count)
        {

        }

        ~LobbyJoinResultPacket() override {}

        template <typename Stream>
        bool serialize(Stream& stream)
        {
            serialize_uint64(stream, m_clientSalt);
            
            serialize_int(stream, m_joined, 0, 1);

            if (m_joined == 1) 
            {
                return true;
            }

            serialize_int(stream, m_reason, 0, Reason::Count);

            return true;
        }

        virtual int32_t getType() const override
        {
            return PacketTypes::LobbyJoinResult;
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

    struct LobbyQueryResultPacket : public Packet
    {
        static const uint32_t MaxLobbyCount = 8;
        uint64_t m_clientSalt; // the sender id
        int32_t m_lobbyCount;
        uint64_t m_lobbySalt[MaxLobbyCount];
        char m_lobbynames[MaxLobbyCount][CreateLobbyPacket::MaxNameLen];

        LobbyQueryResultPacket() :
            m_clientSalt(0),
            m_lobbyCount(0)
        {
            memset(m_lobbySalt, 0, sizeof(m_lobbySalt));
            memset(m_lobbynames, 0, sizeof(m_lobbynames));
        }

        virtual ~LobbyQueryResultPacket() {}

        template <typename Stream>
        bool serialize(Stream& stream)
        {
            serialize_uint64(stream, m_clientSalt);
            serialize_int(stream, m_lobbyCount, 0, MaxLobbyCount);

            for (int32_t i = 0; i < m_lobbyCount; ++i)
            {
                serialize_uint64(stream, m_lobbySalt[i]);
            }
            
            for (int32_t i = 0; i < m_lobbyCount; ++i)
            {
                serialize_string(stream, m_lobbynames[i], CreateLobbyPacket::MaxNameLen);
            }
            return true;
        }

        virtual int32_t getType() const override
        {
            return PacketTypes::LobbyQueryResultPacket;
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