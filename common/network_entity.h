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
	protected:
		NetworkEntity(const NetworkEntityType type) : m_type(type)
		{
		}

		NetworkEntity() = delete;
	private:
		uint32				m_physicsID	{ 0 };
		
		NetworkEntityType	m_type;
	};
}