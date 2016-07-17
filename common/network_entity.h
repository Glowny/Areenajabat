#pragma once

#include "types.h"

namespace arena
{
	enum class NetworkEntityType : uint8
	{
		Null = 0,
		Player,
		Projectile 
	};

	struct NetworkEntity
	{
		bool m_dirty				{ false };

		NetworkEntity()				= default;
	
		virtual NetworkEntityType type() 
		{
			return NetworkEntityType::Null;
		}

		virtual ~NetworkEntity()	= default;
	};
}