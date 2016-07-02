#include "sandbox_scene.h"

#include "..\ecs\entity.h"
#include "..\ecs\sprite_renderer.h"
#include "..\ecs\managers\sprite_manager.h"

#ifdef _DEBUG
#	include <iostream>
#endif

namespace arena
{
	static Entity* ent1 { nullptr };
	static Entity* ent2 { nullptr };

	SandboxSecene::SandboxSecene() : Scene("sandbox")
	{
	}

	void SandboxSecene::onUpdate(const GameTime& time)
	{
		ent1 = Entity::create("ent1");
		ent2 = Entity::create("ent2");
		
		SpriteRenderer* ent1rend = SpriteManager::instance().create(ent1);
	}
	void SandboxSecene::onInitialize()
	{
	}
	void SandboxSecene::onDestroy()
	{
	}
}