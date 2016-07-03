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

        void setFlipX(bool flip);

        void setWeaponAnimation(WeaponAnimationType::Enum type);

        void setStaticContent(TextureResource* crest, TextureResource* helmet, TextureResource* torso, SpriterEngine::EntityInstance* legs);

        void render();
    private:
        Head m_head;
        Torso m_torso;
        Legs m_legs;

        AnimationData* m_animationData;

        WeaponAnimationType::Enum m_weaponAnimType;

        glm::vec2 m_position;

        bool m_flipX;
    };
}
