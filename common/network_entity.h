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
		Scoreboard,
	};

	class NetworkEntity
	{
	public:
		NetworkEntityType type() const
		{
			return m_type;
		}

		// Not all networkentites have PhysicsID, consider removing these.
		void setPhysicsID(const uint32 physicsID)
		{
			m_physicsID = physicsID;
		}
		uint32 getPhysicsID() const
		{
			return m_physicsID;
		}

		void destroy()
		{
			m_remove = true;
		}

		bool getDestroy()
		{
			return m_remove;
		}

		virtual ~NetworkEntity() = default;
		// Remove hax when not drunk
		NetworkEntity(const NetworkEntityType HAXtype, bool haxhax) : m_type(HAXtype)
		{
			haxhax;
		}
		bool				m_hasPhysics = false;
	protected:
		NetworkEntity(const NetworkEntityType type) : m_type(type)
		{
		}

		NetworkEntity() = delete;
		uint32				m_physicsID	{ 666 };
		bool				m_remove = false;

	private:
		
		NetworkEntityType	m_type;
	};
}