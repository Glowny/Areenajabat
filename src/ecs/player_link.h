#pragma once

#include "../rtti/rtti_define.h"
#include "../arena_types.h"
#include "component.h"

namespace arena
{
	class PlayerLink final : public Component
	{
	SET_FRIENDS

	DEFINE_RTTI_SUB_TYPE(PlayerLink)
	
	public:
		
		uint32_t m_playerId;
		~PlayerLink() = default;
		PlayerLink() {  }
		PlayerLink(uint32_t playerId) { m_playerId = playerId; }
	protected:
	};
}
