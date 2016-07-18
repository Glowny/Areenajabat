#include "packet.h"
#include "arena\arena_packet.h"
#include <stdio.h>

namespace arena
{
    Packet* createPacket(int32_t type)
    {
        switch (type)
        {
        case PacketTypes::ConnectionRequest:
            return new ConnectionRequestPacket;
        case PacketTypes::ConnectionDenied:
            return new ConnectionDeniedPacket;
        case PacketTypes::ConnectionChallenge:
            return new ConnectionChallengePacket;
        case PacketTypes::ConnectionResponse:
            return new ConnectionResponsePacket;
        case PacketTypes::KeepAlive:
            return new ConnectionKeepAlivePacket;
        case PacketTypes::Disconnect:
            return new ConnectionDisconnectPacket;

            // master

        case PacketTypes::MasterCreateLobby:
            return new CreateLobbyPacket;
        case PacketTypes::MasterJoinLobby:
            return new JoinLobbyPacket;
        case PacketTypes::MasterListLobbies:
            return new ListLobbiesPacket;
        case PacketTypes::LobbyResultPacket:
            return new LobbyResultPacket;
        case PacketTypes::LobbyQueryResultPacket:
            return new LobbyQueryResultPacket;
        case PacketTypes::LobbyJoinResult:
            return new LobbyJoinResultPacket;

		case PacketTypes::GameSetup:
			return new GameSetupPacket;
		case PacketTypes::GameUpdate:
			return new GameUpdatePacket;
		case PacketTypes::GamePlatform:
			return new GamePlatformPacket;
		case PacketTypes::GameSpawnBullets:
			return new GameSpawnBulletsPacket;
		case PacketTypes::GameBulletHit:
			return new GameBulletHitPacket;
		case PacketTypes::GameDamagePlayer:
			return new GameDamagePlayerPacket;
		case PacketTypes::GameKillPlayer:
			return new GameKillPlayerPacket;
		case PacketTypes::GameRespawnPlayer:
			return new GameRespawnPlayerPacket;
		case PacketTypes::GameUpdateScoreBoard:
			return new GameUpdateScoreBoardPacket;
		case PacketTypes::GameInput:
			return new GameInputPacket;
		case PacketTypes::GameShoot:
			return new GameShootPacket;
		case PacketTypes::GameBulletCurrentPosition:
			return new GameBulletCurrentPositionPacket;
		case PacketTypes::GameSetPlayerAmount:
			return new GameSetPlayerAmountPacket;



        default:
            fprintf(stderr, "Invalid packet type %d", type);
            return nullptr;
        }
    }

    size_t getMaxPacketSize(int32_t type)
    {
        switch (type)
        {
        case PacketTypes::ConnectionRequest:
            return sizeof(ConnectionRequestPacket);
        case PacketTypes::ConnectionDenied:
            return sizeof(ConnectionDeniedPacket);
        case PacketTypes::ConnectionChallenge:
            return sizeof(ConnectionChallengePacket);
        case PacketTypes::ConnectionResponse:
            return sizeof(ConnectionResponsePacket);
        case PacketTypes::KeepAlive:
            return sizeof(ConnectionKeepAlivePacket);
        case PacketTypes::Disconnect:
            return sizeof(ConnectionDisconnectPacket);

            // master

        case PacketTypes::MasterCreateLobby:
            return sizeof( CreateLobbyPacket);
        case PacketTypes::MasterJoinLobby:
            return sizeof( JoinLobbyPacket);
        case PacketTypes::MasterListLobbies:
            return sizeof( ListLobbiesPacket);
        case PacketTypes::LobbyResultPacket:
            return sizeof( LobbyResultPacket);
        case PacketTypes::LobbyQueryResultPacket:
            return sizeof( LobbyQueryResultPacket);
        case PacketTypes::LobbyJoinResult:
            return sizeof( LobbyJoinResultPacket);

        case PacketTypes::GameSetup:
            return sizeof( GameSetupPacket);
        case PacketTypes::GameUpdate:
            return sizeof( GameUpdatePacket);
        case PacketTypes::GamePlatform:
            return sizeof( GamePlatformPacket);
        case PacketTypes::GameSpawnBullets:
            return sizeof( GameSpawnBulletsPacket);
        case PacketTypes::GameBulletHit:
            return sizeof(GameBulletHitPacket);
        case PacketTypes::GameDamagePlayer:
            return sizeof(GameDamagePlayerPacket);
        case PacketTypes::GameKillPlayer:
            return sizeof(GameKillPlayerPacket);
        case PacketTypes::GameRespawnPlayer:
            return sizeof(GameRespawnPlayerPacket);
        case PacketTypes::GameUpdateScoreBoard:
            return sizeof(GameUpdateScoreBoardPacket);
        case PacketTypes::GameInput:
            return sizeof(GameInputPacket);
        case PacketTypes::GameShoot:
            return sizeof(GameShootPacket);
        case PacketTypes::GameBulletCurrentPosition:
            return sizeof(GameBulletCurrentPositionPacket);
        case PacketTypes::GameSetPlayerAmount:
            return sizeof(GameSetPlayerAmountPacket);
        default:
            ARENA_ASSERT(0, "invalid packet type");
            return 0;
        }
    }

    void destroyPacket(Packet* packet)
    {
        delete packet;
    }
}