#pragma once

#include "types.h"

namespace arena
{
	enum class NetworkEntityType : uint8
	{
		Null = 0,
		Player,
		Projectile,
		Weapon,
		Gladiator
	};

	class NetworkEntity
	{
	public:
		NetworkEntityType type() const
		{
			return m_type;
		}

		virtual ~NetworkEntity() = default;
	protected:
		NetworkEntity(const NetworkEntityType type) : m_type(type)
		{
		}

		NetworkEntity() = delete;
	private:
		NetworkEntityType m_type;
	};
}