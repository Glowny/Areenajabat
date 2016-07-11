#include "packet.h"
#include "areena\areena_packet.h"
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
            return nullptr;
        case PacketTypes::KeepAlive:
            return nullptr;
        case PacketTypes::Disconnect:
            return nullptr;
		case PacketTypes::GameSetup:
			return new GameSetupPacket;
		case PacketTypes::GameUpdate:
			return new GameUpdatePacket;
		case PacketTypes::GamePlatform:
			return new GamePlaformPacket;
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
		case PacketTypes::GameMovement:
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
}