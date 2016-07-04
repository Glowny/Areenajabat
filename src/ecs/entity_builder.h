#pragma once

#include "..\3rdparty\glm\vec2.hpp"
#include "..\forward_declare.h"
#include "..\arena_types.h"
#include "..\rect.h"

FORWARD_DECLARE_1(FORWARD_DECLARE_TYPE_CLASS, arena, TextureResource)
FORWARD_DECLARE_1(FORWARD_DECLARE_TYPE_CLASS, arena, SpriteRenderer)
FORWARD_DECLARE_1(FORWARD_DECLARE_TYPE_CLASS, arena, Transform)
FORWARD_DECLARE_1(FORWARD_DECLARE_TYPE_CLASS, arena, Entity)

namespace arena
{
	class EntityBuilder final
	{
	public:
		EntityBuilder()		= default;

		void begin();

		Transform* const addTransformComponent();
		SpriteRenderer* const addSpriteRenderer();
		
		void addTag(const String& tag);
		void setTags(const String& tags);

		Entity* getResults();

		~EntityBuilder()	= default;
	private:
		Entity* m_entity { nullptr };
	};
}