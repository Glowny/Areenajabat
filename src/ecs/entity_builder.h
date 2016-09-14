#pragma once

#include <glm/glm.hpp>
#include "../forward_declare.h"
#include "../arena_types.h"
#include "../rect.h"
#include "id.h"

FORWARD_DECLARE_1(FORWARD_DECLARE_TYPE_CLASS, arena, TextureResource)
FORWARD_DECLARE_1(FORWARD_DECLARE_TYPE_CLASS, arena, SpriteRenderer)
FORWARD_DECLARE_1(FORWARD_DECLARE_TYPE_CLASS, arena, Transform)
FORWARD_DECLARE_1(FORWARD_DECLARE_TYPE_CLASS, arena, Entity)
FORWARD_DECLARE_1(FORWARD_DECLARE_TYPE_CLASS, arena, Movement)
FORWARD_DECLARE_1(FORWARD_DECLARE_TYPE_CLASS, arena, Timer)
FORWARD_DECLARE_1(FORWARD_DECLARE_TYPE_CLASS, arena, PhysicsComponent)
FORWARD_DECLARE_1(FORWARD_DECLARE_TYPE_CLASS, arena, Projectile)
FORWARD_DECLARE_1(FORWARD_DECLARE_TYPE_CLASS, arena, BulletTrail)
FORWARD_DECLARE_1(FORWARD_DECLARE_TYPE_CLASS, arena, Animator)
FORWARD_DECLARE_1(FORWARD_DECLARE_TYPE_CLASS, arena, BulletModel)
FORWARD_DECLARE_1(FORWARD_DECLARE_TYPE_CLASS, arena, CharacterComponent)
namespace arena
{

	class EntityBuilder final
	{
	public:
		EntityBuilder()		= default;

		void begin();

		Transform* const addTransformComponent();
		SpriteRenderer* const addSpriteRenderer();
        Animator* const addCharacterAnimator();
		Timer* const addTimer();
		Movement* const addMovement();
		Id* const addIdentifier();
		Id* const addIdentifier(EntityIdentification id);
		Projectile* const addProjectile();
		PhysicsComponent* const addPhysicsComponent();
		BulletTrail* const addBulletTrail();
		CharacterComponent* const addCharacterComponent();
		void addTag(const String& tag);
		void setTags(const String& tags);
		Entity* getResults();
		~EntityBuilder()	= default;
	private:
		Entity* m_entity { nullptr };
	};
}
