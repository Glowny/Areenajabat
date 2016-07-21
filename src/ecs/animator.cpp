#include "animator.h"

namespace arena
{
    REGISTER_RTTI_SUB_TYPE(Animator)


    void Animator::rotateAimTo(float radians)
    {
        m_animator.rotateAimTo(radians);
    }
}