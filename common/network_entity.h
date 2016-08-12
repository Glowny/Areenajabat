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
		void setEntityID(const uint8_t entityID)
		{
			m_entityID = entityID;
		}
		uint8_t getEntityID() const
		{
			return m_entityID;
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
			m_remove = false;
		}
		bool				m_hasPhysics = false;
	protected:
		NetworkEntity(const NetworkEntityType type) : m_type(type)
		{
			m_remove = false;
		}

		NetworkEntity() = delete;
		uint8_t				m_entityID{ 255 };
		bool				m_remove = false;

	private:
		
		NetworkEntityType	m_type;
	};
}