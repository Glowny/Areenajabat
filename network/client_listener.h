#pragma once

#include <bx/bx.h>

namespace arena
{
    struct BX_NO_VTABLE ClientListener
    {
        virtual ~ClientListener() = 0;
        virtual void onClientConnected(uint32_t clientIndex, ENetPeer* from, double timestamp) = 0;
    };

    inline ClientListener::~ClientListener() {}
}