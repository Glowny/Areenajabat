#pragma once

#include "../network_entity.h"
#include "../mem/memory.h"
#include "playerController.h"
#include "gladiator.h"

namespace arena
{
	class Player final : public NetworkEntity
	{
	public:
		uint32					m_clientIndex{ NULL };
		PlayerController*		m_playerController{ nullptr };
		Gladiator*				m_gladiator{ nullptr };

		Player() : NetworkEntity(NetworkEntityType::Player)
		{
			m_remove = false;
		}

		bool operator ==(const Player* const lhs) const
		{
			if (lhs == nullptr) return false;

			return ADDRESSOF(lhs) == ADDRESSOF(this);
		}
		bool operator ==(const Player& lhs) const
		{
			return this == &lhs;
		}
		bool operator !=(const Player* const lhs) const
		{
			return !(lhs == this);
		}
	};
}