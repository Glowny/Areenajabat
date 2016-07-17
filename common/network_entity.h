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
		NetworkEntity()				= default;
	
		virtual NetworkEntityType type() 
		{
			return NetworkEntityType::Null;
		}

		virtual ~NetworkEntity()	= default;
	};
}