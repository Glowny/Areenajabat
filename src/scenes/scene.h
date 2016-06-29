#pragma once

#include "..\ecs\entity.h"
#include "..\forward_declare.h"
#include "..\arena_types.h"
#include "..\functional.h"

FORWARD_DECLARE_1(FORWARD_DECLARE_TYPE_CLASS, arena, GameTime)
FORWARD_DECLARE_1(FORWARD_DECLARE_TYPE_CLASS, arena, Entity)

using EntityIterator = std::vector<arena::Entity*>::iterator;


namespace arena
{
	enum class SceneState : uint8
	{
		Uninitialized = 0,
		Active,
		Suspended,
		Destroyed 
	};
	
	class Scene
	{
	public:
		const String& name() const;
		SceneState state() const;

		void activate();
		void deactivate();
		void destroy();

		void update(const GameTime& gameTime);

		EntityIterator entititesBegin();
		EntityIterator entititesEnd();

		Entity* const find(Predicate<Entity* const> pred);

		void registerEntity(Entity* const entity);
		void unregisterEntity(Entity* const entity);

		virtual ~Scene();
	protected:
		Scene(const String& name);

		virtual void onUpdate(const GameTime& time);
		virtual void onInitialize();
		virtual void onResume();
		virtual void onSuspend();
		virtual void onDestroy();
	private:
		const String			m_name;

		std::vector<Entity*>	m_entities;
		
		SceneState				m_state;
	};
}