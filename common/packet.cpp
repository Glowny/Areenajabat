#include "packet.h"
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
            return nullptr;
        default:
            fprintf(stderr, "Invalid packet type %d", type);
            return nullptr;
        }
    }
}