#include "network_interface.h"
#include <enet\enet.h>
#include "debug.h"

namespace arena
{
    static bool s_initialized = false;

    bool networkInitialize()
    {
        ARENA_ASSERT(s_initialized == false, "Network already initialized");

        int result = enet_initialize();

        if (result == 0)
        {
            s_initialized = true;
        }

        return s_initialized;
    }

    void networkShutdown()
    {
        ARENA_ASSERT(s_initialized == true, "Network not initialized");
        enet_deinitialize();

        s_initialized = false;
    }
}