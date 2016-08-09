#pragma once
#include <glm/vec2.hpp>
#include "weapons.h"

namespace arena
{
    struct PlayerInput
    { 
		PlayerInput& operator=(const PlayerInput& input)
		{
			m_leftButtonDown = input.m_leftButtonDown;
			m_rightButtonDown = input.m_rightButtonDown;
			m_upButtonDown = input.m_upButtonDown;
			m_downButtonDown = input.m_downButtonDown;
			// if copied without check the jump/shoot messages may be missed.
			if (!m_jumpButtonDown)
			{
				m_jumpButtonDown = input.m_jumpButtonDown;
			}
			if(!m_shootButtonDown)
			{ 
				m_shootButtonDown = input.m_shootButtonDown;
			}
			if (!m_grenadeButtonDown)
			{
				m_grenadeButtonDown = input.m_grenadeButtonDown;
			}
			if (!m_reloadButtonDown)
			{
				m_reloadButtonDown = input.m_reloadButtonDown;
			}
			m_changeWeaponButtonDown = input.m_changeWeaponButtonDown;
			return *this;
		}

        PlayerInput() :
            m_leftButtonDown(false),
            m_rightButtonDown(false),
            m_upButtonDown(false),
            m_downButtonDown(false),
            m_jumpButtonDown(false),
            m_shootButtonDown(false),
            m_grenadeButtonDown(false),
            m_changeWeaponButtonDown(false),
			m_reloadButtonDown(false)
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
		bool m_reloadButtonDown;
    };

	struct PlayerController
	{
        PlayerController()
        {

        }

        PlayerInput m_input;
		float aimAngle;
	};
}
