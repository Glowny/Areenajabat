#pragma once

#include "..\mem\heap_allocator.h"
#include "..\forward_declare.h"
#include "..\rtti\rtti_define.h"
#include "..\arena_types.h"

FORWARD_DECLARE_1(FORWARD_DECLARE_TYPE_CLASS, arena, Entity)
FORWARD_DECLARE_1(FORWARD_DECLARE_TYPE_CLASS, arena, SpriteManager)

#define COMPONENT_INITIALIZE(__ptr__, __type__, ...) DYNAMIC_NEW(__ptr__, __type__, ...)

namespace arena
{
	class Component
	{
	DEFINE_RTTI_SUPER_TYPE(Component)
	public:
		Entity* owner();

		void destroy();
		bool destroyed() const;

		virtual ~Component();
	protected:
		Component(Entity* owner);

		virtual void onDestroy();
	private:
		Entity* m_owner;

		bool m_destroyed;
	};
}