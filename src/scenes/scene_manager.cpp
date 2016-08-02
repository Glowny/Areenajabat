#include "../game_time.h"
#include "scene_manager.h"
#include "scene.h"

namespace arena
{
	void SceneManager::push(Scene* const scene)
	{
		m_scenes.push(scene);
	}
	Scene* const SceneManager::pop()
	{
		Scene* const scene = m_scenes.top();
		m_scenes.pop();

		return scene;
	}
	Scene* const SceneManager::peek()
	{
		return m_scenes.top();
	}

	void SceneManager::update(const GameTime& gameTime)
	{
		if (m_scenes.empty()) return;

		m_scenes.top()->update(gameTime);
	}

	SceneManager& SceneManager::instance()
	{
		static SceneManager sceneManager;

		return sceneManager;
	}
}