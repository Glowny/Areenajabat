#pragma once
#include "../packet.h"
#include <glm/glm.hpp>
#include "../types.h"
#include "playerController.h"

namespace arena
{
//TODO: Set size on how many bullet creation events can possibly happen
#define BULLET_MAX_AMOUNT 30
	
	struct BulletData
	{
		BulletData()
		{
			m_ownerId = 0;
			m_id = 0;
			m_type = 0;
			m_position = glm::vec2(0,0);
			m_rotation = 0;
			m_creationDelay = 0;
		}
		uint8_t m_ownerId;
		uint8_t m_id;
		uint8_t m_type;
		glm::vec2 m_position;
		float m_rotation;
		float m_creationDelay; // TODO: Replace with gametime later.
	}; 

	// TODO: Remove unused variables when clientside physics is done.
	struct BulletHitData
	{
		BulletHitData()
		{
			m_id = 0;
			m_type = 0;
			m_position = glm::vec2(0, 0);
			m_rotation = 0;
			m_creationDelay = 0;
		}
		uint8_t m_id;
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
			m_aimAngle = 0;
		}
		uint8_t m_id;
		glm::vec2 m_position;
		glm::vec2 m_velocity;
		uint8_t m_team;
		float m_aimAngle;
		uint32_t m_ownerId;
		int8_t m_climbing;
		bool m_throwing;
		bool m_reloading;

	};
	struct PlayerScoreData
	{
		int m_score;
		int m_tickets;
		int m_kills;
		int m_playerID;
	};

	struct ScoreBoardData
	{
		PlayerScoreData m_playerScoreArray[CHARACTER_MAXAMOUNT];
		uint8_t m_flagHolder;

	};

	// TODO: Raise this.
#define PLATFORM_VERTEX_MAXAMOUNT 30
	struct PlatformData
	{
		PlatformData()
		{
			m_type = 0;
		}
		uint8_t m_type;
		int32_t m_vertexAmount;
		glm::vec2 m_vertexArray[PLATFORM_VERTEX_MAXAMOUNT];

	};

    struct GameSetupPacket : public Packet
    {
        static const int32_t MaxPlayers = 32;

        uint64_t m_clientSalt;
		uint8_t m_clientIndex;
		int32_t m_playerAmount;

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
            serialize_int(stream, m_playerAmount, 0, MaxPlayers);
			serialize_bytes(stream, &m_clientIndex, 1);
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
				
				serialize_uint32(stream, m_characterArray[i].m_ownerId);
				serialize_float(stream, m_characterArray[i].m_position.x);
				serialize_float(stream, m_characterArray[i].m_position.y);
				serialize_float(stream, m_characterArray[i].m_velocity.x);
				serialize_float(stream, m_characterArray[i].m_velocity.y);
				serialize_float(stream, m_characterArray[i].m_aimAngle);
				serialize_bool(stream, m_characterArray[i].m_throwing);
				serialize_bool(stream, m_characterArray[i].m_reloading);
				serialize_int(stream, m_characterArray[i].m_climbing, 0, 2);
				serialize_bytes(stream, &m_characterArray[i].m_team, 1);
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

	struct GameCreateGladiatorsPacket : public Packet
	{
		uint64_t m_clientSalt;
		CharacterData m_characterArray[CHARACTER_MAXAMOUNT];
		int32_t m_playerAmount;

		GameCreateGladiatorsPacket()
			: m_clientSalt(0)
		{
			m_playerAmount = 0;
		}

		virtual ~GameCreateGladiatorsPacket() {}

		template <typename Stream>
		bool serialize(Stream& stream)
		{
			serialize_uint64(stream, m_clientSalt);

			serialize_int(stream, m_playerAmount, 0, CHARACTER_MAXAMOUNT);
			for (int32_t i = 0; i < m_playerAmount; ++i)
			{

				serialize_uint32(stream, m_characterArray[i].m_ownerId);
				serialize_bytes(stream, &m_characterArray[i].m_id, 1);
				serialize_float(stream, m_characterArray[i].m_position.x);
				serialize_float(stream, m_characterArray[i].m_position.y);
				serialize_float(stream, m_characterArray[i].m_velocity.x);
				serialize_float(stream, m_characterArray[i].m_velocity.y);
				serialize_float(stream, m_characterArray[i].m_aimAngle);
				serialize_bytes(stream, &m_characterArray[i].m_team, 1);

			}
			return true;
		}

		virtual int32_t getType() const override
		{
			return PacketTypes::GameCreateGladiators;
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

	struct GamePlatformPacket : public Packet
	{
		uint64_t m_clientSalt;
		PlatformData m_platform;

		GamePlatformPacket()
			: m_clientSalt(0)
		{
			m_platform.m_vertexAmount = 0;
			m_platform.m_type = 0;
		}

		virtual ~GamePlatformPacket() {}

		template <typename Stream>
		bool serialize(Stream& stream)
		{
			serialize_uint64(stream, m_clientSalt);
			serialize_bytes(stream, &m_platform.m_type, 1);
			serialize_int(stream, m_platform.m_vertexAmount, 0, PLATFORM_VERTEX_MAXAMOUNT);
			for (int32_t i = 0; i < m_platform.m_vertexAmount; ++i)
			{
				serialize_float(stream, m_platform.m_vertexArray[i].x);
				serialize_float(stream, m_platform.m_vertexArray[i].y);
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
			m_bulletAmount = 0;
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
				serialize_bytes(stream, &m_bulletSpawnArray[i].m_id, 1);
				serialize_bytes(stream, &m_bulletSpawnArray[i].m_ownerId, 1);
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
		BulletHitData bulletHitArray[BULLET_MAX_AMOUNT];
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
				serialize_bytes(stream, &bulletHitArray[i].m_id, 1);
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
		int32 m_damageAmount;
		uint8_t m_hitDirection; // Direction where the bullet came from, bool
		uint8_t m_bulletId;
		glm::vec2 m_hitPosition;

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
			serialize_bytes(stream, &m_bulletId, 1);
			serialize_bytes(stream, &m_hitDirection, 1);
			serialize_int(stream, m_damageAmount, 0, 100);
			serialize_float(stream, m_hitPosition.x);
			serialize_float(stream, m_hitPosition.y);
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
			serialize_bytes(stream, &m_scoreBoardData.m_flagHolder, 1);
			for (unsigned i = 0; i < m_playerAmount; ++i)
			{
				serialize_int(stream, m_scoreBoardData.m_playerScoreArray[i].m_score, 0, 1023);
				serialize_int(stream, m_scoreBoardData.m_playerScoreArray[i].m_kills, 0, 127);
				serialize_int(stream, m_scoreBoardData.m_playerScoreArray[i].m_tickets, 0, 127);
				serialize_int(stream, m_scoreBoardData.m_playerScoreArray[i].m_playerID, 0, 127);
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
		uint64 m_clientSalt;
		uint64 m_challengeSalt;

        PlayerInput m_input;

		float m_aimAngle;

		GameInputPacket() : 
            m_clientSalt(0),
            m_challengeSalt(0),

            m_aimAngle(0.f)
		{
		}

		virtual ~GameInputPacket() {}

		template <typename Stream>
		bool serialize(Stream& stream)
		{
			serialize_uint64(stream, m_clientSalt);
            serialize_uint64(stream, m_challengeSalt);
			serialize_float(stream, m_aimAngle);
            serialize_bool(stream, m_input.m_leftButtonDown);
            serialize_bool(stream, m_input.m_rightButtonDown);
            serialize_bool(stream, m_input.m_upButtonDown);
            serialize_bool(stream, m_input.m_downButtonDown);
            serialize_bool(stream, m_input.m_jumpButtonDown);
            serialize_bool(stream, m_input.m_shootButtonDown);
            serialize_bool(stream, m_input.m_grenadeButtonDown);
            serialize_bool(stream, m_input.m_changeWeaponButtonDown);
			serialize_bool(stream, m_input.m_reloadButtonDown);
			
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
        uint64_t m_clientSalt;
		uint64_t m_challengeSalt;
		float m_angle; // Less accurate data could be send (data send is between (0-360)).

		GameShootPacket() : 
            m_clientSalt(0),
            m_challengeSalt(0),
            m_angle(0.f)
		{

		}

		virtual ~GameShootPacket() {}

		template <typename Stream>
		bool serialize(Stream& stream)
		{
			serialize_uint64(stream, m_clientSalt);
            serialize_uint64(stream, m_challengeSalt);
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

	struct GameRequestMapPacket : public Packet
	{
		uint64_t m_clientSalt;
		uint64_t m_challengeSalt;
		uint8_t mapID; // Less accurate data could be send (data send is between (0-360)).

		GameRequestMapPacket() :
			m_clientSalt(0),
			m_challengeSalt(0),
			mapID(0)
		{

		}

		virtual ~GameRequestMapPacket() {}

		template <typename Stream>
		bool serialize(Stream& stream)
		{
			serialize_uint64(stream, m_clientSalt);
			serialize_uint64(stream, m_challengeSalt);
			serialize_bytes(stream, &mapID, 1);
			return true;
		}

		virtual int32_t getType() const override
		{
			return PacketTypes::GameRequestMap;
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

	struct GameModePacket : public Packet
	{
		uint64_t m_clientSalt;
		int32_t m_gameModeIndex;

		GameModePacket()
			: m_clientSalt(0)
		{
			m_gameModeIndex = 0;
		}

		virtual ~GameModePacket() {}

		template <typename Stream>
		bool serialize(Stream& stream)
		{
			serialize_uint64(stream, m_clientSalt);

			serialize_int(stream, m_gameModeIndex, 0, 10);

			return true;
		}

		virtual int32_t getType() const override
		{
			return PacketTypes::GameMode;
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
