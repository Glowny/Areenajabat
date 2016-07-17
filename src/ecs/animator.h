#pragma once

#include "../rtti/rtti_define.h"
#include "component.h"
#include "../graphics/character_animator.h"

namespace arena
{
    class Animator final : public Component
    {
        SET_FRIENDS;

        DEFINE_RTTI_SUB_TYPE(Animator);
    public:
        ~Animator() = default;
    protected:
        Animator() = default;
    public:
        CharacterAnimator m_animator;
    };
}