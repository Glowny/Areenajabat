#pragma once

#include <glm/glm.hpp>
#include "..\forward_declare.h"
#include "..\arena_types.h"
#include "..\rect.h"
#include "..\ecs\timer.h"
#include "..\ecs\movement.h"
FORWARD_DECLARE_1(FORWARD_DECLARE_TYPE_CLASS, arena, TextureResource)
FORWARD_DECLARE_1(FORWARD_DECLARE_TYPE_CLASS, arena, SpriteRenderer)
FORWARD_DECLARE_1(FORWARD_DECLARE_TYPE_CLASS, arena, Transform)
FORWARD_DECLARE_1(FORWARD_DECLARE_TYPE_CLASS, arena, Entity)
FORWARD_DECLARE_1(FORWARD_DECLARE_TYPE_CLASS, arena, Animator)

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

		void addTag(const String& tag);
		void setTags(const String& tags);

		Entity* getResults();

		~EntityBuilder()	= default;
	private:
		Entity* m_entity { nullptr };
	};
}