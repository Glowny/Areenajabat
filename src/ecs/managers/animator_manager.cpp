#include "animator_manager.h"
#include "../transform.h"
#include "../entity.h"

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
            Component* const tx = /*(Transform* const)*/owner->first(TYPEOF(Transform)); (void)tx;
            CharacterAnimator& animator = (*it)->m_animator;

            //animator.setPosition(tx->m_position);
            animator.update(gameTime.m_delta);
            animator.render();
        }
    }
}