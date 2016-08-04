#pragma once

namespace SpriterEngine { class EntityInstance; }

#include <glm/glm.hpp>
#include <string>

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

		void startResumePlayback();
		void pausePlayback();
        void setTimeElapsed(double milliseconds);
		void setCurrentTime(double milliseconds);
        void render();

        void setCurrentAnimation(const std::string& anim);
        std::string getCurrentAnimationName();
        void setCurrentAnimation(int anim);

        double getCurrentTime() const;

        bool isFinished() const;
    public:
        SpriterEngine::EntityInstance* m_entity;
    };
}

