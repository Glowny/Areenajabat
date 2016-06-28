#include "scene_impl_base.h"

#include <algorithm>
#include <cassert>

namespace arena
{
	const String& SceneImplBase::name() const
	{
		return m_name;
	}

	SceneImplBase::SceneImplBase(const String& name) : m_name(name)
	{
	}

	void SceneImplBase::registerEntity(Entity* const entity)
	{
#if _DEBUG
		// "Safe" add only in debug. Aka 
		// check if the entity is already 
		// registered.
		auto iter = std::find_if(m_entities.begin(), m_entities.end(), [entity](Entity* const e)
		{
			return e == entity;
		});

		assert(iter == m_entities.end());
#endif

		m_entities.push_back(entity);
	}
	void SceneImplBase::unregisterEntity(Entity* const entity)
	{
		m_entities.erase(std::find(m_entities.begin(), m_entities.end(), entity), m_entities.end());
	}

	EntityIterator SceneImplBase::entititesBegin()
	{
		return m_entities.begin();
	}
	EntityIterator SceneImplBase::entititesEnd()
	{
		return m_entities.end();
	}

	Entity* const SceneImplBase::find(Predicate<Entity* const> pred)
	{
		for (Entity* entity : m_entities) if (pred(entity)) return entity;

		return nullptr;
	}
}