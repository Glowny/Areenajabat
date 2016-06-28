#pragma once

#include "..\forward_declare.h"
#include "..\arena_types.h"

FORWARD_DECLARE_1(FORWARD_DECLARE_TYPE_CLASS, arena, GameTime)

namespace arena
{
	struct SceneState final
	{
		enum Enum : uint8
		{
			Uninitialized = 0,
			Active,
			Suspended,
			Destroyed 
		};
	};

	template<typename T>
	class Scene
	{
	public:
		Scene(T* impl);

		const String& name() const;
		SceneState state() const;

		void activate();
		void deactivate();
		void destroy();

		void update(const GameTime& gameTime);
		void draw(const GameTime& gameTime);
	private:
		T*			m_impl;

		SceneState	m_state;
	};
}