#pragma once

#include "..\mem\heap_allocator.h"
#include "..\forward_declare.h"
#include "..\rtti\rtti_define.h"
#include "..\arena_types.h"

FORWARD_DECLARE_1(FORWARD_DECLARE_TYPE_CLASS, arena, Entity)

namespace arena
{
	class Component
	{
	DEFINE_RTTI_TYPE
	public:
		/*
			Static members.
		*/
		static Component* create(Entity* owner);

		/*
			Instance members.
		*/
		Component(Entity* owner);	// TODO: hide this.

		Entity* owner();

		void destroy();

		virtual ~Component();
	private:
		/*
			Static members.
		*/
		static HeapAllocator s_allocator;

		/*
			Instance members.
		*/
		Entity* m_owner;
	};
}