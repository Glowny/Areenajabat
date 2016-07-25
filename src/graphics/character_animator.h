#pragma once

#include "../res/spriter_animation_player.h"
#include "composite_sprite.h"
#include "animation_system.h"

namespace arena
{

    struct Head
    {
        Head() :
            m_helmet(nullptr),
            m_crest(nullptr)
        {
            
        }

        CompositeSprite m_helmet;
        CompositeSprite m_crest;
    };

    struct Legs
    {
        Legs() : m_animation(nullptr) {}

        SpriterAnimationPlayer m_animation;
        glm::vec2 m_relativeOffset;
    };

	struct Death
	{
		Death() : m_animation(nullptr) 
		{
			dying = false;
			m_relativeOffset = glm::vec2(0, 0);
		}

		SpriterAnimationPlayer m_animation;
		glm::vec2 m_relativeOffset;
		bool dying;
	};

    struct Torso
    {
        Torso() : m_sprite(nullptr) {}

        CompositeSprite m_sprite;
        glm::vec2 m_relativeOffset;
    };

    class CharacterAnimator
    {
    public:
        CharacterAnimator();
        
        void update(float dt);

        void setPosition(const glm::vec2& position);

        glm::vec2& getPosition();

        const glm::vec2& getPosition() const;

        void setFlipX(bool flip);

        void setWeaponAnimation(WeaponAnimationType::Enum type);

		void playDeathAnimation(bool hitDirection, float hitPositionY);

        void setStaticContent(TextureResource* crest, TextureResource* helmet, TextureResource* torso,
			SpriterEngine::EntityInstance* legs, SpriterEngine::EntityInstance* death);

        void render();

        void rotateAimTo(float radians);
    private:
        Head m_head;
        Torso m_torso;
        Legs m_legs;
		Death m_death;
        AnimationData* m_animationData;

        WeaponAnimationType::Enum m_weaponAnimType;

        glm::vec2 m_position;

        bool m_flipX;
		float aimAngle;
    };
}
