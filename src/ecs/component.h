#pragma once

#include "../mem/heap_allocator.h"
#include "../forward_declare.h"
#include "../rtti/rtti_define.h"
#include "../arena_types.h"

FORWARD_DECLARE_1(FORWARD_DECLARE_TYPE_CLASS, arena, Entity)
FORWARD_DECLARE_1(FORWARD_DECLARE_TYPE_CLASS, arena, SpriteManager)

#define SET_FRIENDS template<typename T> \
					friend class ComponentManager;

namespace arena
{
	class Component
	{
	friend class Entity;

	SET_FRIENDS
	
	DEFINE_RTTI_SUPER_TYPE(Component)
	
	public:
		Entity* owner();

		void destroy();
		bool destroyed() const;

		virtual ~Component();
	protected:
		Component();

		virtual void onDestroy();
	private:
		Entity* m_owner;

		bool m_destroyed;
	};
}