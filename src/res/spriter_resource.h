#pragma once
#include <string>
#include <spriterengine/model/spritermodel.h>

namespace SpriterEngine { class EntityInstance; }
namespace arena
{
    class SpriterResource
    {
    public:
        SpriterResource(const std::string&);
        SpriterEngine::EntityInstance* getNewEntityInstance(int entityId);
        SpriterEngine::EntityInstance* getNewEntityInstance(std::string entityName);
    private:
        SpriterEngine::SpriterModel m_model;
    };

    namespace spriter
    {
        void* load(const std::string name);
        void unload(void*);
    }
}

