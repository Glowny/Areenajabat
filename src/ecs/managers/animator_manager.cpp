#include "animator_manager.h"
#include "../transform.h"
#include "../entity.h"
#include <common/debug.h>

namespace arena
{

    AnimatorManager::AnimatorManager() : ComponentManager()
    {
    }

    void AnimatorManager::onUpdate(const GameTime& gameTime) 
    {
        auto it = begin();
        auto last = end();

        for (; it != last; ++it)
        {
            Entity* owner = (*it)->owner();
            Transform* const tx = (Transform* const)owner->first(TYPEOF(Transform));

            ARENA_ASSERT(tx != nullptr, "Transform can not be null");

            CharacterAnimator& animator = (*it)->m_animator;

            animator.setPosition(tx->m_position);
            animator.update(gameTime.m_delta);
            animator.render();
        }
    }
}