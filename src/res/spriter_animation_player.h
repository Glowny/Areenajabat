#pragma once

namespace SpriterEngine { class EntityInstance; }

#include <glm/glm.hpp>

namespace arena
{
    class SpriterAnimationPlayer
    {
    public:
        SpriterAnimationPlayer(SpriterEngine::EntityInstance* instance);
        void setPosition(const glm::vec2& position);
        void setAngle(float radians);
        void setScale(const glm::vec2& scale);
        void setAlpha(float alpha);
        
        glm::vec2 getPosition() const;
        glm::vec2 getScale() const;
        float getAngle() const;
        float getAlpha() const;

        void setTimeElapsed(double milliseconds);
        void render();

        void setCurrentAnimation(const std::string& anim);
        void setCurrentAnimation(int anim);
    private:
        SpriterEngine::EntityInstance* m_entity;
    };
}

