#include "sandbox_scene.h"
#include "..\res\resource_manager.h"
#include "..\app.h"

#include "..\ecs\entity.h"
#include "..\ecs\sprite_renderer.h"
#include "..\ecs\managers\sprite_manager.h"

#include "..\game_time.h"

#ifdef _DEBUG
#	include <iostream>
#   include "..\rtti\rtti_define.h"
#   include "..\ecs\transform.h"
#   include "..\ecs\sprite_renderer.h"
#endif

namespace arena
{
	static Entity* ent1 { nullptr };
	static Entity* ent2 { nullptr };

	SandboxSecene::SandboxSecene() : Scene("sandbox")
	{
	}

	static void test_RTTI()
	{
		const auto transt = TYPEOF(Transform);
		const auto spritt = TYPEOF(SpriteRenderer);
		const auto rendtt = TYPEOF(RenderComponent);
		const auto comptt = TYPEOF(Component);

		volatile static int j = 0;

		if (j == 0) return;
	}

	void SandboxSecene::onUpdate(const GameTime&)
	{
	}
	void SandboxSecene::onInitialize()
	{
		test_RTTI();

		ent1 = Entity::create("ent1");
		ent2 = Entity::create("ent2");
		
		SpriteRenderer* ent1rend = SpriteManager::instance().create(ent1);
		ent1rend->setTexture(App::instance().resources()->get<TextureResource>(ResourceType::Texture, "perkele.png"));
		ent1->add(ent1rend);
		SpriteManager::instance().registerComponent(ent1rend);
	}
	void SandboxSecene::onDestroy()
	{
	}
}