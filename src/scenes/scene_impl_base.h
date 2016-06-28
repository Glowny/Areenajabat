#pragma once

#include "..\functional.h"
#include "..\forward_declare.h"
#include "..\arena_types.h"

#include <vector>

FORWARD_DECLARE_1(FORWARD_DECLARE_TYPE_CLASS, arena, Entity)

using EntityIterator = std::vector<arena::Entity>::iterator;

namespace arena
{
	class SceneImplBase
	{
	public:
		const String& name() const;

		/*
			Public hooks we can use with scene.

			void activate();
			void suspend();
			void resume();
			void destroy();

			void update(const GameTime& gameTime);
			void draw(const GameTime& gameTime);
		*/

		virtual ~SceneImplBase() = default;
	protected:
		SceneImplBase(const String& name);

		void registerEntity(Entity* const entity);
		void unregisterEntity(Entity* const entity);

		EntityIterator entititesBegin();
		EntityIterator entititesEnd();

		Entity* const find(Predicate<Entity* const> pred);
	private:
		std::vector<Entity*> entities;

		const String m_name;
	};
}