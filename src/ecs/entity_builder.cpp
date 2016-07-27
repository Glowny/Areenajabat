#include "entity_builder.h"

#include "entity.h"
#include "transform.h"
#include "sprite_renderer.h"
#include "..\res\texture_resource.h"
#include "managers\transform_manager.h"
#include "managers\sprite_manager.h"
#include "managers/animator_manager.h"
#include <cassert>
#include <common/debug.h>

namespace arena
{
	void EntityBuilder::begin()
	{
		m_entity = Entity::create();
	}

	Transform* const EntityBuilder::addTransformComponent()
	{
		ARENA_ASSERT(m_entity != nullptr, "Entity cannot be null");

		Transform* const transform = TransformManager::instance().create();

		m_entity->add(transform);

		TransformManager::instance().registerComponent(transform);

		return transform;
	}
	SpriteRenderer* const EntityBuilder::addSpriteRenderer()
	{
        ARENA_ASSERT(m_entity != nullptr, "Entity cannot be null");

		SpriteRenderer* const renderer = SpriteManager::instance().create();

		m_entity->add(renderer);

		SpriteManager::instance().registerComponent(renderer);

		return renderer;
	}

    Animator* const EntityBuilder::addCharacterAnimator()
    {
        AnimatorManager& instance = AnimatorManager::instance();
        Animator* const anim = instance.create();

        m_entity->add(anim);

        instance.registerComponent(anim);

        return anim;
    }

	Timer* const EntityBuilder::addTimer()
	{
		Timer* timer = new Timer();
		m_entity->add(timer);
		return timer;
	}

	void EntityBuilder::addTag(const String& tag)
	{
        ARENA_ASSERT(m_entity != nullptr, "Entity cannot be null");

		m_entity->setTags(m_entity->getTags() + "|" + tag);
	}
	void EntityBuilder::setTags(const String& tags)
	{
        ARENA_ASSERT(m_entity != nullptr, "Entity cannot be null");

		m_entity->setTags(tags);
	}

	Entity* EntityBuilder::getResults()
	{
		Entity* results = m_entity;
		
		m_entity = nullptr;

		return results;
	}
}