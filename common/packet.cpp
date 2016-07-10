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
            return nullptr;
        case PacketTypes::ConnectionChallenge:
            return nullptr;
        case PacketTypes::ConnectionResponse:
            return nullptr;
        case PacketTypes::KeepAlive:
            return nullptr;
        case PacketTypes::Disconnect:
            return nullptr;
        default:
            fprintf(stderr, "Invalid packet type %d", type);
            return nullptr;
        }
    }
}