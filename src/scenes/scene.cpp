#include "../ecs/entity.h"
#include "scene.h"

#include <algorithm>

namespace arena
{
	const String& Scene::name() const
	{
		return m_name;
	}
	SceneState Scene::state() const
	{
		return m_state;
	}

	void Scene::activate()
	{
		switch (m_state)
		{
		case SceneState::Uninitialized:
			onInitialize();
			break;
		case SceneState::Suspended:
			onResume();
			break;
		default:
			// Destroyed, active.
			return;
		}

		m_state = SceneState::Active;
	}
	void Scene::deactivate()
	{
		switch (m_state)
		{
		case SceneState::Active:
			onSuspend();
			break;
		default:
			// Destroyed, suspended, uninitialized.
			return;
		}

		m_state = SceneState::Suspended;
	}
	void Scene::destroy()
	{
		if (m_state == SceneState::Uninitialized || m_state == SceneState::Destroyed) return;

		onDestroy();
	}

	void Scene::update(const GameTime& gameTime)
	{
		onUpdate(gameTime);

        auto begin = m_entities.begin();
        for (int32_t i = (int32_t)m_entities.size() - 1; i >= 0; --i)
        {
            if (m_entities[i]->destroyed())
                m_entities.erase(begin + i);
        }
	}
	
	void Scene::registerEntity(Entity* const entity)
	{
#if _DEBUG
		// "Safe" add only in debug. Aka 
		// check if the entity is already 
		// registered.
		auto iter = std::find_if(m_entities.begin(), m_entities.end(), [entity](Entity* const e)
		{
			return e == entity;
		});

		//assert(iter == m_entities.end());
#endif

		m_entities.push_back(entity);
	}
	void Scene::unregisterEntity(Entity* const entity)
	{
		m_entities.erase(std::find(m_entities.begin(), m_entities.end(), entity), m_entities.end());
	}

	EntityIterator Scene::entititesBegin()
	{
		return m_entities.begin();
	}
	EntityIterator Scene::entititesEnd()
	{
		return m_entities.end();
	}

	Entity* const Scene::find(Predicate<Entity* const> pred)
	{
		for (Entity* entity : m_entities) if (pred(entity)) return entity;

		return nullptr;
	}

	/*
		Internal event impls.
	*/

	void Scene::onUpdate(const GameTime& gameTime)
	{
        BX_UNUSED(gameTime);
	}
	void Scene::onInitialize()
	{
	}
	void Scene::onResume()
	{
	}
	void Scene::onSuspend()
	{
	}
	void Scene::onDestroy()
	{
	}

	Scene::~Scene()
	{
		for (auto it = entititesBegin(); it != entititesEnd(); it++)
		{
			Entity* const entity = *it;
			
			entity->destroy();
		}
	}

	Scene::Scene(const String& name) :  m_name(name),
										m_state(SceneState::Uninitialized)
	{
	}
}