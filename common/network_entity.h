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
		Gladiator,
		Map,
		BulletHit,
		RespawnPlayer,
	};

	class NetworkEntity
	{
	public:
		NetworkEntityType type() const
		{
			return m_type;
		}

		void setPhysicsID(const uint32 physicsID)
		{
			m_physicsID = physicsID;
		}
		uint32 getPhysicsID() const
		{
			return m_physicsID;
		}

		virtual ~NetworkEntity() = default;
		// Remove hax when not drunk
		NetworkEntity(const NetworkEntityType HAXtype, bool haxhax) : m_type(HAXtype)
		{
			haxhax;
		}
	protected:
		NetworkEntity(const NetworkEntityType type) : m_type(type)
		{
		}

		NetworkEntity() = delete;
		uint32				m_physicsID	{ 0 };
	private:
		
		NetworkEntityType	m_type;
	};
}