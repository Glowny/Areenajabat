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

#include "..\ecs\entity_builder.h"

namespace arena
{
	static Entity* entity;

	SandboxSecene::SandboxSecene() : Scene("sandbox")
	{
	}

	void SandboxSecene::onUpdate(const GameTime&)
	{
	}
	void SandboxSecene::onInitialize()
	{
		EntityBuilder builder;

		builder.begin();
		
		SpriteRenderer* renderer = builder.addSpriteRenderer();
		renderer->setTexture(App::instance().resources()->get<TextureResource>(ResourceType::Texture, "perkele.png"));
		renderer->anchor();

		Transform* transform = builder.addTransformComponent();
		transform->m_position.x = 0.0f;
		transform->m_position.y = 0.0f;

		entity = builder.getResults();
	
		registerEntity(entity);
	}
	void SandboxSecene::onDestroy()
	{
	}
}