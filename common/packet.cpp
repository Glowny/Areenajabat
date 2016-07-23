#include "packet.h"
#include "mem\block_allocator.h"
#include "arena\arena_packet.h"
#include <stdio.h>

namespace arena
{
	const uint32 InitialSingleBlockSize = 32;
	const uint32 InitialBlocksCount		= 32;

	static BlockAllocator s_allocator(InitialSingleBlockSize, InitialBlocksCount);

	static const uint32 s_packetSizes[] 
	{
		sizeof(ConnectionRequestPacket),
		sizeof(ConnectionDeniedPacket),
		sizeof(ConnectionChallengePacket),
		sizeof(ConnectionResponsePacket),
		sizeof(ConnectionKeepAlivePacket),
		sizeof(ConnectionDisconnectPacket),
		sizeof(CreateLobbyPacket),
		sizeof(JoinLobbyPacket),
		sizeof(ListLobbiesPacket),
		sizeof(LobbyResultPacket),
		sizeof(LobbyQueryResultPacket),
		sizeof(LobbyJoinResultPacket),
		sizeof(GameSetupPacket),
		sizeof(GameUpdatePacket),
		sizeof(GameCreateGladiatorsPacket),
		sizeof(GamePlatformPacket),
		sizeof(GameSpawnBulletsPacket),
		sizeof(GameBulletHitPacket),
		sizeof(GameDamagePlayerPacket),
		sizeof(GameKillPlayerPacket),
		sizeof(GameRespawnPlayerPacket),
		sizeof(GameUpdateScoreBoardPacket),
		sizeof(GameInputPacket),
		sizeof(GameShootPacket),
		sizeof(GameBulletCurrentPositionPacket),
		sizeof(GameSetPlayerAmountPacket)
	};

#if _DEBUG
	static uint32 createCalls	= 0;
	static uint32 destroyCalls	= 0;
#endif

    Packet* createPacket(int32_t type)
    {
#if _DEBUG
		createCalls++;

		if (createCalls == 38) {
			volatile static int i = 7;
			i++;
		}
#endif

		Packet* packet = reinterpret_cast<Packet*>(s_allocator.allocate(getMaxPacketSize(type)));

        switch (type)
        {
        case PacketTypes::ConnectionRequest:
            DYNAMIC_NEW_DEFAULT(packet, ConnectionRequestPacket);
			break;
        case PacketTypes::ConnectionDenied:
            DYNAMIC_NEW_DEFAULT(packet, ConnectionDeniedPacket);
			break;
        case PacketTypes::ConnectionChallenge:
            DYNAMIC_NEW_DEFAULT(packet, ConnectionChallengePacket);
			break;
        case PacketTypes::ConnectionResponse:
            DYNAMIC_NEW_DEFAULT(packet, ConnectionResponsePacket);
			break;
        case PacketTypes::KeepAlive:
            DYNAMIC_NEW_DEFAULT(packet, ConnectionKeepAlivePacket);
			break;
        case PacketTypes::Disconnect:
            DYNAMIC_NEW_DEFAULT(packet, ConnectionDisconnectPacket);
			break;

            // master

        case PacketTypes::MasterCreateLobby:
            DYNAMIC_NEW_DEFAULT(packet, CreateLobbyPacket);
			break;
        case PacketTypes::MasterJoinLobby:
            DYNAMIC_NEW_DEFAULT(packet, JoinLobbyPacket);
			break;
        case PacketTypes::MasterListLobbies:
            DYNAMIC_NEW_DEFAULT(packet, ListLobbiesPacket);
			break;
        case PacketTypes::LobbyResultPacket:
            DYNAMIC_NEW_DEFAULT(packet, LobbyResultPacket);
			break;
        case PacketTypes::LobbyQueryResultPacket:
            DYNAMIC_NEW_DEFAULT(packet, LobbyQueryResultPacket);
			break;
        case PacketTypes::LobbyJoinResult:
            DYNAMIC_NEW_DEFAULT(packet, LobbyJoinResultPacket);
			break;

		case PacketTypes::GameSetup:
			DYNAMIC_NEW_DEFAULT(packet, GameSetupPacket);
			break;
		case PacketTypes::GameUpdate:
			DYNAMIC_NEW_DEFAULT(packet, GameUpdatePacket);
			break;
		case PacketTypes::GameCreateGladiators:
			DYNAMIC_NEW_DEFAULT(packet, GameCreateGladiatorsPacket);
			break;
		case PacketTypes::GamePlatform:
			DYNAMIC_NEW_DEFAULT(packet, GamePlatformPacket);
			break;
		case PacketTypes::GameSpawnBullets:
			DYNAMIC_NEW_DEFAULT(packet, GameSpawnBulletsPacket);
			break;
		case PacketTypes::GameBulletHit:
			DYNAMIC_NEW_DEFAULT(packet, GameBulletHitPacket);
			break;
		case PacketTypes::GameDamagePlayer:
			DYNAMIC_NEW_DEFAULT(packet, GameDamagePlayerPacket);
			break;
		case PacketTypes::GameKillPlayer:
			DYNAMIC_NEW_DEFAULT(packet, GameKillPlayerPacket);
			break;
		case PacketTypes::GameRespawnPlayer:
			DYNAMIC_NEW_DEFAULT(packet, GameRespawnPlayerPacket);
			break;
		case PacketTypes::GameUpdateScoreBoard:
			DYNAMIC_NEW_DEFAULT(packet, GameUpdateScoreBoardPacket);
			break;
		case PacketTypes::GameInput:
			DYNAMIC_NEW_DEFAULT(packet, GameInputPacket);
		case PacketTypes::GameShoot:
			DYNAMIC_NEW_DEFAULT(packet, GameShootPacket);
			break;
		case PacketTypes::GameBulletCurrentPosition:
			DYNAMIC_NEW_DEFAULT(packet, GameBulletCurrentPositionPacket);
			break;
		case PacketTypes::GameSetPlayerAmount:
			DYNAMIC_NEW_DEFAULT(packet, GameSetPlayerAmountPacket);
			break;
        default:
            fprintf(stderr, "Invalid packet type %d", type);
			
			s_allocator.deallocate(reinterpret_cast<Char* const>(packet), getMaxPacketSize(type));

			packet = nullptr;

			break;
		 }

		return packet;
	}

    size_t getMaxPacketSize(int32_t type)
    {
		#pragma region Old impl
        //switch (type)
        //{
        //case PacketTypes::ConnectionRequest:
        //    return sizeof(ConnectionRequestPacket);
        //case PacketTypes::ConnectionDenied:
        //    return sizeof(ConnectionDeniedPacket);
        //case PacketTypes::ConnectionChallenge:
        //    return sizeof(ConnectionChallengePacket);
        //case PacketTypes::ConnectionResponse:
        //    return sizeof(ConnectionResponsePacket);
        //case PacketTypes::KeepAlive:
        //    return sizeof(ConnectionKeepAlivePacket);
        //case PacketTypes::Disconnect:
        //    return sizeof(ConnectionDisconnectPacket);

        //    // master

        //case PacketTypes::MasterCreateLobby:
        //    return sizeof(CreateLobbyPacket);
        //case PacketTypes::MasterJoinLobby:
        //    return sizeof(JoinLobbyPacket);
        //case PacketTypes::MasterListLobbies:
        //    return sizeof(ListLobbiesPacket);
        //case PacketTypes::LobbyResultPacket:
        //    return sizeof(LobbyResultPacket);
        //case PacketTypes::LobbyQueryResultPacket:
        //    return sizeof(LobbyQueryResultPacket);
        //case PacketTypes::LobbyJoinResult:
        //    return sizeof(LobbyJoinResultPacket);
        //case PacketTypes::GameSetup:
        //    return sizeof(GameSetupPacket);
        //case PacketTypes::GameUpdate:
        //    return sizeof(GameUpdatePacket);
        //case PacketTypes::GamePlatform:
        //    return sizeof(GamePlatformPacket);
        //case PacketTypes::GameSpawnBullets:
        //    return sizeof(GameSpawnBulletsPacket);
        //case PacketTypes::GameBulletHit:
        //    return sizeof(GameBulletHitPacket);
        //case PacketTypes::GameDamagePlayer:
        //    return sizeof(GameDamagePlayerPacket);
        //case PacketTypes::GameKillPlayer:
        //    return sizeof(GameKillPlayerPacket);
        //case PacketTypes::GameRespawnPlayer:
        //    return sizeof(GameRespawnPlayerPacket);
        //case PacketTypes::GameUpdateScoreBoard:
        //    return sizeof(GameUpdateScoreBoardPacket);
        //case PacketTypes::GameInput:
        //    return sizeof(GameInputPacket);
        //case PacketTypes::GameShoot:
        //    return sizeof(GameShootPacket);
        //case PacketTypes::GameBulletCurrentPosition:
        //    return sizeof(GameBulletCurrentPositionPacket);
        //case PacketTypes::GameSetPlayerAmount:
        //    return sizeof(GameSetPlayerAmountPacket);
        //default:
        //    ARENA_ASSERT(0, "invalid packet type");
        //    return 0;
        //}
#pragma endregion

		if (type >= PacketTypes::Count) return 0;

		return s_packetSizes[type];
    }

    void destroyPacket(Packet* packet)
    {
#if _DEBUG
		destroyCalls++;
#endif

		s_allocator.deallocate(reinterpret_cast<Char* const>(packet), getMaxPacketSize(packet->getType()));
	
		const int32 type = packet->getType();

		switch (type)
		{
		case PacketTypes::ConnectionRequest:
			DYNAMIC_DTOR(static_cast<ConnectionRequestPacket*>(packet), ConnectionRequestPacket);
			break;
		case PacketTypes::ConnectionDenied:
			DYNAMIC_DTOR(static_cast<ConnectionDeniedPacket*>(packet), ConnectionDeniedPacket);
			break;
		case PacketTypes::ConnectionChallenge:
			DYNAMIC_DTOR(static_cast<ConnectionChallengePacket*>(packet), ConnectionChallengePacket);
			break;
		case PacketTypes::ConnectionResponse:
			DYNAMIC_DTOR(static_cast<ConnectionResponsePacket*>(packet), ConnectionResponsePacket);
			break;
		case PacketTypes::KeepAlive:
			DYNAMIC_DTOR(static_cast<ConnectionKeepAlivePacket*>(packet), ConnectionKeepAlivePacket);
			break;
		case PacketTypes::Disconnect:
			DYNAMIC_DTOR(static_cast<ConnectionDisconnectPacket*>(packet), ConnectionDisconnectPacket);
			break;

			// master

		case PacketTypes::MasterCreateLobby:
			DYNAMIC_DTOR(static_cast<CreateLobbyPacket*>(packet), CreateLobbyPacket);
			break;
		case PacketTypes::MasterJoinLobby:
			DYNAMIC_DTOR(static_cast<JoinLobbyPacket*>(packet), JoinLobbyPacket);
			break;
		case PacketTypes::MasterListLobbies:
			DYNAMIC_DTOR(static_cast<ListLobbiesPacket*>(packet), ListLobbiesPacket);
			break;
		case PacketTypes::LobbyResultPacket:
			DYNAMIC_DTOR(static_cast<LobbyResultPacket*>(packet), LobbyResultPacket);
			break;
		case PacketTypes::LobbyQueryResultPacket:
			DYNAMIC_DTOR(static_cast<LobbyQueryResultPacket*>(packet), LobbyQueryResultPacket);
			break;
		case PacketTypes::LobbyJoinResult:
			DYNAMIC_DTOR(static_cast<LobbyJoinResultPacket*>(packet), LobbyJoinResultPacket);
			break;

		case PacketTypes::GameSetup:
			DYNAMIC_DTOR(static_cast<GameSetupPacket*>(packet), GameSetupPacket);
			break;
		case PacketTypes::GameUpdate:
			DYNAMIC_DTOR(static_cast<GameUpdatePacket*>(packet), GameUpdatePacket);
			break;
		case PacketTypes::GameCreateGladiators:
			DYNAMIC_DTOR(static_cast<GameCreateGladiatorsPacket*>(packet), GameCreateGladiatorsPacket);
			break;
		case PacketTypes::GamePlatform:
			DYNAMIC_DTOR(static_cast<GamePlatformPacket*>(packet), GamePlatformPacket);
			break;
		case PacketTypes::GameSpawnBullets:
			DYNAMIC_DTOR(static_cast<GameSpawnBulletsPacket*>(packet), GameSpawnBulletsPacket);
			break;
		case PacketTypes::GameBulletHit:
			DYNAMIC_DTOR(static_cast<GameBulletHitPacket*>(packet), GameBulletHitPacket);
			break;
		case PacketTypes::GameDamagePlayer:
			DYNAMIC_DTOR(static_cast<GameDamagePlayerPacket*>(packet), GameDamagePlayerPacket);
			break;
		case PacketTypes::GameKillPlayer:
			DYNAMIC_DTOR(static_cast<GameKillPlayerPacket*>(packet), GameKillPlayerPacket);
			break;
		case PacketTypes::GameRespawnPlayer:
			DYNAMIC_DTOR(static_cast<GameRespawnPlayerPacket*>(packet), GameRespawnPlayerPacket);
			break;
		case PacketTypes::GameUpdateScoreBoard:
			DYNAMIC_DTOR(static_cast<GameUpdateScoreBoardPacket*>(packet), GameUpdateScoreBoardPacket);
			break;
		case PacketTypes::GameInput:
			DYNAMIC_DTOR(static_cast<GameInputPacket*>(packet), GameInputPacket);
			break;
		case PacketTypes::GameShoot:
			DYNAMIC_DTOR(static_cast<GameShootPacket*>(packet), GameShootPacket);
			break;
		case PacketTypes::GameBulletCurrentPosition:
			DYNAMIC_DTOR(static_cast<GameBulletCurrentPositionPacket*>(packet), GameBulletCurrentPositionPacket);
			break;
		case PacketTypes::GameSetPlayerAmount:
			DYNAMIC_DTOR(static_cast<GameSetPlayerAmountPacket*>(packet), GameSetPlayerAmountPacket);
			break;
		default:
			fprintf(stderr, "Invalid packet type %d", type);
		}
	}
}