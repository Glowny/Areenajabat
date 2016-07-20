#pragma once
#include <glm\vec2.hpp>
#include "weapons.h"

namespace arena
{
    struct PlayerInput
    {
        PlayerInput() :
            m_leftButtonDown(false),
            m_rightButtonDown(false),
            m_upButtonDown(false),
            m_downButtonDown(false),
            m_jumpButtonDown(false),
            m_shootButtonDown(false),
            m_grenadeButtonDown(false),
            m_changeWeaponButtonDown(false)
        {

        }

        bool m_leftButtonDown;
        bool m_rightButtonDown;
        bool m_upButtonDown;
        bool m_downButtonDown;
        bool m_jumpButtonDown;
        bool m_shootButtonDown;
        bool m_grenadeButtonDown;
        bool m_changeWeaponButtonDown;
    };

	struct PlayerController
	{
        PlayerController()
        {

        }

        PlayerInput m_input;

		float m_jumpDirection;
		bool shootFlag;
		bool moveFlag;
		float aimAngle;
		WeaponType weapon;
	};
}