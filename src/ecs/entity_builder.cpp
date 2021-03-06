#include "entity_builder.h"

#include "entity.h"
#include "transform.h"
#include "sprite_renderer.h"
#include "timer.h"
#include "movement.h"
#include "projectile.h"
#include "physics_component.h"
#include "bullet_trail.h"
#include "../res/texture_resource.h"
#include "managers/transform_manager.h"
#include "managers/sprite_manager.h"
#include "managers/animator_manager.h"
#include "managers/projectile_manager.h"
#include "managers/physics_manager.h"
#include "managers/trail_manager.h"
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

	Movement* const EntityBuilder::addMovement()
	{
		Movement* movement = new Movement();
		m_entity->add(movement);
		return movement;
	}

	Id* const EntityBuilder::addIdentifier()
	{
		Id* id = new Id();
		m_entity->add(id);
		return id;
	}
	Id* const EntityBuilder::addIdentifier(EntityIdentification id)
	{
		Id* identification = new Id();
		identification->m_id = id;
		m_entity->add(identification);
		return identification;
	}
	Projectile* const EntityBuilder::addProjectile()
	{
		ProjectileManager& instance = ProjectileManager::instance();
		Projectile* const projectile = instance.create();
		instance.registerComponent(projectile);
		m_entity->add(projectile);
		return projectile;
	}
	PhysicsComponent* const EntityBuilder::addPhysicsComponent()
	{
		PhysicsManager& instance = PhysicsManager::instance();
		PhysicsComponent* const component = instance.create();
		instance.registerComponent(component);
		m_entity->add(component);
		return component;
	}
	BulletTrail* const EntityBuilder::addBulletTrail()
	{
		BulletTrail* trail = new BulletTrail();
		m_entity->add(trail);
		TrailManager::instance().registerComponent(trail);
		return trail;
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