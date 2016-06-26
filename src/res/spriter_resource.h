#pragma once
#include <string>
#include <spriterengine/model/spritermodel.h>

namespace arena
{
    class SpriterResource
    {
    public:
        SpriterResource(const std::string&);
    public:
        SpriterEngine::SpriterModel m_model;
    };

    namespace spriter
    {
        void* load(const std::string name);
        void unload(void*);
    }
}

