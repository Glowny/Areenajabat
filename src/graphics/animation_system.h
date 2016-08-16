#pragma once
#include <bx/macros.h>

namespace arena
{
    class CompositeSprite;
    struct BX_NO_VTABLE IHandAnimation
    {
        virtual ~IHandAnimation() = 0;
        virtual void create() = 0;
        virtual void flip() = 0;
        virtual void rotateTo(float radians) = 0;
		virtual void setDirection(bool direction) = 0;
        virtual CompositeSprite* getParent() = 0;
    };
    inline IHandAnimation::~IHandAnimation() {}

    struct BX_NO_VTABLE IWeaponAnimation
    {

    };

    struct AnimationData
    {
        IHandAnimation* m_rightHand;
        IHandAnimation* m_leftHand;
        IWeaponAnimation* m_reload;
    };

    struct WeaponAnimationType
    {
        enum Enum
        {
            Gladius,
            Count
        };
    };

    void animationSystemInit();
    void animationSystemShutdown();

    AnimationData* getAnimationDataFor(WeaponAnimationType::Enum type);

    
}
