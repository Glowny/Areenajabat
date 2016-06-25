#pragma once
#include <string>

namespace arena
{
    class SpriterResource
    {

    };

    namespace spriter
    {
        void* load(const std::string name);
        void unload(void*);
    }
}

