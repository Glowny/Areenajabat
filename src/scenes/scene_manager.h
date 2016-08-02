#pragma once

#include "../forward_declare.h"

#include <stack>

FORWARD_DECLARE_1(FORWARD_DECLARE_TYPE_CLASS, arena, GameTime)
FORWARD_DECLARE_1(FORWARD_DECLARE_TYPE_CLASS, arena, Scene)

using SceneStack = std::stack<arena::Scene*>;

namespace arena
{
	class SceneManager final
	{
	public:
		static SceneManager& instance();

		void push(Scene* const scene);
		Scene* const pop();
		Scene* const peek();

		void update(const GameTime& gameTime);

		~SceneManager() = default;
	
		SceneManager(SceneManager const& copy)			  = delete;
		SceneManager& operator=(SceneManager const& copy) = delete;
	private:
		SceneManager() = default;
		
		SceneStack m_scenes;
	};
}
