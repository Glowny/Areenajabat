#pragma once

#include "component_manager.h"
#include "../animator.h"
#include "../../singleton.h"
#include "../../game_time.h"

namespace arena
{
    class AnimatorManager final : public ComponentManager<Animator>
    {
    public:
        SET_FRIENDS
        MAKE_SINGLETON(AnimatorManager)
    protected:
        virtual void onUpdate(const GameTime& gameTime) final override;

        //virtual void onRegister(Component* const component) final override;
    };
}