#pragma once
#include "../packet.h"
#include <glm\glm.hpp>


namespace arena
{
//TODO: Set size on how many bullet creation events can possibly happen
#define BULLET_MAX_AMOUNT 50
	struct BulletData
	{
		BulletData()
		{
			m_type = 0;
			m_position = glm::vec2(0,0);
			m_rotation = 0;
			m_creationDelay = 0;
		}
		uint8_t m_type;
		glm::vec2 m_position;
		float m_rotation;
		float m_creationDelay; // TODO: Replace with gametime later.
	};

#define CHARACTER_MAXAMOUNT 12	
	struct CharacterData
	{
		CharacterData()
		{
			m_position = glm::vec2(0,0);
			m_velocity = glm::vec2(0, 0);
			m_rotation = 0;
		}
		glm::vec2 m_position;
		glm::vec2 m_velocity;
		float m_rotation;

	};
	struct PlayerScoreData
	{
		int m_score;
		int m_tickets;

	};

	struct ScoreBoardData
	{
		PlayerScoreData m_playerScoreArray[CHARACTER_MAXAMOUNT];
		uint8_t m_flagHolder;

	};

#define PLATFORM_POINTS_MAXAMOUNT 50
	struct PlatformData
	{
		PlatformData()
		{
			m_type = 0;
		}
		uint8_t m_type;
		uint8_t m_pointAmount;
		glm::vec2 m_pointArray[PLATFORM_POINTS_MAXAMOUNT];

	};

    struct GameSetupPacket : public Packet
    {
        uint64_t m_clientSalt;
		uint8_t m_playerAmount;

		GameSetupPacket()
            : m_clientSalt(0)
        {
			m_playerAmount = 0;
        }

        virtual ~GameSetupPacket() {}

        template <typename Stream>
        bool serialize(Stream& stream)
        {
            serialize_uint64(stream, m_clientSalt);
			serialize_bytes(stream,& m_playerAmount, 1);
			return true;
        }

        virtual int32_t getType() const override
        {
            return PacketTypes::GameSetup;
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

	struct GameUpdatePacket : public Packet
	{
		uint64_t m_clientSalt;
		CharacterData m_characterArray[CHARACTER_MAXAMOUNT];
		int32_t m_playerAmount;

		GameUpdatePacket()
			: m_clientSalt(0)
		{
			m_playerAmount = 0;
		}

		virtual ~GameUpdatePacket() {}

		template <typename Stream>
		bool serialize(Stream& stream)
		{
			serialize_uint64(stream, m_clientSalt);

			serialize_int(stream, m_playerAmount, 0, CHARACTER_MAXAMOUNT);
			for (int32_t i = 0; i < m_playerAmount; ++i)
			{
				serialize_float(stream, m_characterArray[i].m_position.x);
				serialize_float(stream, m_characterArray[i].m_position.y);
				serialize_float(stream, m_characterArray[i].m_velocity.x);
				serialize_float(stream, m_characterArray[i].m_velocity.y);
				serialize_float(stream, m_characterArray[i].m_rotation);
				
			}
			return true;
		}

		virtual int32_t getType() const override
		{
			return PacketTypes::GameUpdate;
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

	struct GamePlaformPacket : public Packet
	{
		uint64_t m_clientSalt;
		PlatformData m_platform;

		GamePlaformPacket()
			: m_clientSalt(0)
		{
			m_platform.m_pointAmount = 0;
			m_platform.m_type = 0;
		}

		virtual ~GamePlaformPacket() {}

		template <typename Stream>
		bool serialize(Stream& stream)
		{
			serialize_uint64(stream, m_clientSalt);
			serialize_bytes(stream, &m_platform.m_type, 1); // TODO, CHECK HOW TO BYTES
			serialize_int(stream, m_platform.m_pointAmount, 0, PLATFORM_POINTS_MAXAMOUNT);
			for (unsigned i = 0; i < m_platform.m_pointAmount; ++i)
			{
				serialize_float(stream, m_platform.m_pointArray[i].x);
				serialize_float(stream, m_platform.m_pointArray[i].y);
			}
			return true;
		}

		virtual int32_t getType() const override
		{
			return PacketTypes::GamePlatform;
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

	struct GameSpawnBulletsPacket : public Packet
	{
		uint64_t m_clientSalt;
		uint8_t m_bulletAmount;
		BulletData m_bulletSpawnArray[BULLET_MAX_AMOUNT];
		GameSpawnBulletsPacket()
			: m_clientSalt(0)
		{

		}

		virtual ~GameSpawnBulletsPacket() {}

		template <typename Stream>
		bool serialize(Stream& stream)
		{
			serialize_uint64(stream, m_clientSalt);
			serialize_int(stream, m_bulletAmount, 0, BULLET_MAX_AMOUNT);
			for (unsigned i = 0; i < m_bulletAmount; ++i)
			{
				serialize_bytes(stream, &m_bulletSpawnArray[i].m_type, 1);
				serialize_float(stream, m_bulletSpawnArray[i].m_position.x);
				serialize_float(stream, m_bulletSpawnArray[i].m_position.y);
				serialize_float(stream, m_bulletSpawnArray[i].m_rotation);
			}
			return true;
		}

		virtual int32_t getType() const override
		{
			return PacketTypes::GameSpawnBullets;
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

	struct GameBulletHitPacket : public Packet
	{
		uint64_t m_clientSalt;
		uint8_t m_bulletAmount;
		BulletData bulletHitArray[BULLET_MAX_AMOUNT];
		GameBulletHitPacket()
			: m_clientSalt(0)
		{

		}

		virtual ~GameBulletHitPacket() {}

		template <typename Stream>
		bool serialize(Stream& stream)
		{
			serialize_uint64(stream, m_clientSalt);
			serialize_int(stream, m_bulletAmount, 0, BULLET_MAX_AMOUNT);
			for (unsigned i = 0; i < m_bulletAmount; ++i)
			{
				serialize_bytes(stream, &bulletHitArray[i].m_type, 1);
				serialize_float(stream, bulletHitArray[i].m_position.x);
				serialize_float(stream, bulletHitArray[i].m_position.y);
				serialize_float(stream, bulletHitArray[i].m_rotation);
			}
			return true;
		}

		virtual int32_t getType() const override
		{
			return PacketTypes::GameBulletHit;
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

	struct GameDamagePlayerPacket : public Packet
	{
		uint64_t m_clientSalt;
		uint8_t m_targetID;
		float m_damageAmount;

		GameDamagePlayerPacket()
			: m_clientSalt(0)
		{
			m_targetID = 0;
			m_damageAmount = 0;
		}

		virtual ~GameDamagePlayerPacket() {}

		template <typename Stream>
		bool serialize(Stream& stream)
		{
			serialize_bytes(stream, &m_targetID, 1);
			serialize_float(stream, m_damageAmount);
			serialize_uint64(stream, m_clientSalt);
			return true;
		}

		virtual int32_t getType() const override
		{
			return PacketTypes::GameDamagePlayer;
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

	struct GameKillPlayerPacket : public Packet
	{
		uint64_t m_clientSalt;
		uint8_t m_playerID;
		GameKillPlayerPacket()
			: m_clientSalt(0)
		{

		}

		virtual ~GameKillPlayerPacket() {}

		template <typename Stream>
		bool serialize(Stream& stream)
		{
			serialize_uint64(stream, m_clientSalt);
			serialize_bytes(stream, &m_playerID, 1);
			return true;
		}

		virtual int32_t getType() const override
		{
			return PacketTypes::GameKillPlayer;
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

	struct GameRespawnPlayerPacket : public Packet
	{
		uint64_t m_clientSalt;
		uint8_t m_playerID;

		GameRespawnPlayerPacket()
			: m_clientSalt(0)
		{

		}

		virtual ~GameRespawnPlayerPacket() {}

		template <typename Stream>
		bool serialize(Stream& stream)
		{
			serialize_uint64(stream, m_clientSalt);
			serialize_bytes(stream, &m_playerID, 1);

			return true;
		}

		virtual int32_t getType() const override
		{
			return PacketTypes::GameRespawnPlayer;
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

	struct GameUpdateScoreBoardPacket : public Packet
	{
		uint64_t m_clientSalt;
		ScoreBoardData m_scoreBoardData;
		uint8_t m_playerAmount;
		GameUpdateScoreBoardPacket()
			: m_clientSalt(0)
		{

		}

		virtual ~GameUpdateScoreBoardPacket() {}

		template <typename Stream>
		bool serialize(Stream& stream)
		{
			serialize_uint64(stream, m_clientSalt);
			serialize_int(stream, m_playerAmount, 0, CHARACTER_MAXAMOUNT);
			for (unsigned i = 0; i < m_playerAmount; ++i)
			{
				serialize_int(stream, m_scoreBoardData.m_playerScoreArray[i].m_score, 0, 1000);
				serialize_int(stream, m_scoreBoardData.m_playerScoreArray[i].m_tickets, 0, 1000);
				serialize_bytes(stream, &m_scoreBoardData.m_flagHolder, 1);
			}
			return true;
		}

		virtual int32_t getType() const override
		{
			return PacketTypes::GameUpdateScoreBoard;
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


	struct GameInputPacket : public Packet
	{
		unsigned m_id;
		uint64_t m_clientSalt;
		uint8_t x;
		uint8_t y;
		GameInputPacket()
			: m_clientSalt(0)
		{
			x = 0;
			y = 0;
		}

		virtual ~GameInputPacket() {}

		template <typename Stream>
		bool serialize(Stream& stream)
		{
			serialize_uint64(stream, m_clientSalt);
			serialize_bytes(stream, &x, 1);
			serialize_bytes(stream, &y, 1);
			return true;
		}

		virtual int32_t getType() const override
		{
			return PacketTypes::GameInput;
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

	struct GameShootPacket : public Packet
	{
		unsigned m_id;
		uint64_t m_clientSalt;
		float m_angle; // Less accurate data could be send (data send is between (0-360)).

		GameShootPacket()
			: m_clientSalt(0)
		{

		}

		virtual ~GameShootPacket() {}

		template <typename Stream>
		bool serialize(Stream& stream)
		{
			serialize_uint64(stream, m_clientSalt);
			serialize_float(stream, m_angle);
			return true;
		}

		virtual int32_t getType() const override
		{
			return PacketTypes::GameShoot;
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

	struct GameBulletCurrentPositionPacket : public Packet
	{
		uint64_t m_clientSalt;
		BulletData m_bulletArray[BULLET_MAX_AMOUNT];
		uint8_t m_bulletAmount;
		

		GameBulletCurrentPositionPacket()
			: m_clientSalt(0)
		{
			
		}

		virtual ~GameBulletCurrentPositionPacket() {}

		template <typename Stream>
		bool serialize(Stream& stream)
		{
			serialize_uint64(stream, m_clientSalt);
			serialize_int(stream, m_bulletAmount, 0, BULLET_MAX_AMOUNT);
			for (unsigned i = 0; i < m_bulletAmount; ++i)
			{
				serialize_float(stream, m_bulletArray[i].m_position.x);
				serialize_float(stream, m_bulletArray[i].m_position.y);
			}
			return true;
		}

		virtual int32_t getType() const override
		{
			return PacketTypes::GameBulletCurrentPosition;
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

	struct GameSetPlayerAmountPacket : public Packet
	{
		uint64_t m_clientSalt;
		uint8_t m_playerAmount;
		GameSetPlayerAmountPacket()
			: m_clientSalt(0)
		{

		}

		virtual ~GameSetPlayerAmountPacket() {}

		template <typename Stream>
		bool serialize(Stream& stream)
		{
			serialize_uint64(stream, m_clientSalt);
			serialize_bytes(stream, &m_playerAmount, 1);
			return true;
		}

		virtual int32_t getType() const override
		{
			return PacketTypes::GameSetPlayerAmount;
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