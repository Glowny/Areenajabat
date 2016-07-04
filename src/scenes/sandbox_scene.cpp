#include "sandbox_scene.h"
#include "..\res\resource_manager.h"
#include "..\app.h"

#include "..\ecs\entity.h"
#include "..\ecs\sprite_renderer.h"
#include "..\ecs\managers\sprite_manager.h"

#include "..\game_time.h"

#include "../graphics/character_animator.h"
#include "../res/spriter_resource.h"
#include "../ecs/animator.h"

#ifdef _DEBUG
#	include <iostream>
#   include "..\rtti\rtti_define.h"
#   include "..\ecs\transform.h"
#   include "..\ecs\sprite_renderer.h"
#endif

#include "..\ecs\entity_builder.h"
#include "../ecs/managers/animator_manager.h"
#include <bx/fpumath.h>
#include <glm/gtc/type_ptr.hpp>
#include "../graphics/spritebatch.h"

namespace arena
{
	static Entity* entity;

	SandboxSecene::SandboxSecene() : Scene("sandbox")
	{
	}

	void SandboxSecene::onUpdate(const GameTime& gameTime)
	{

        Camera& camera = App::instance().camera();
        camera.calculate();
        // set
        float ortho[16];
        bx::mtxOrtho(ortho, 0.0f, float(camera.m_bounds.x), float(camera.m_bounds.y), 0.0f, 0.0f, 1000.0f);
        bgfx::setViewTransform(0, glm::value_ptr(camera.m_matrix), ortho);
        bgfx::setViewRect(0, 0, 0, uint16_t(camera.m_bounds.x), uint16_t(camera.m_bounds.y));

        bgfx::dbgTextClear();

        SpriteManager::instance().update(gameTime);
        AnimatorManager::instance().update(gameTime);
        

        App::instance().spriteBatch()->submit(0);
	}

	void SandboxSecene::onInitialize()
	{
		EntityBuilder builder;

		builder.begin();
		
        ResourceManager* resources = App::instance().resources();

		SpriteRenderer* renderer = builder.addSpriteRenderer();
		renderer->setTexture(resources->get<TextureResource>(ResourceType::Texture, "perkele.png"));
		renderer->anchor();

		Transform* transform = builder.addTransformComponent();
		transform->m_position.x = 0.0f;
		transform->m_position.y = 0.0f;


        Animator* const animator = builder.addCharacterAnimator();
        CharacterAnimator& anim = animator->m_animator;
        anim.setStaticContent(
            resources->get<TextureResource>(ResourceType::Texture, "Characters/head/1_Crest.png"),
            resources->get<TextureResource>(ResourceType::Texture, "Characters/head/1_Helmet.png"),
            resources->get<TextureResource>(ResourceType::Texture, "Characters/body/1_Torso.png"),
            resources->get<SpriterResource>(ResourceType::Spriter, "Characters/Animations/LegAnimations/Run.scml")->getNewEntityInstance(0)
        );
        anim.setWeaponAnimation(WeaponAnimationType::Gladius);
        anim.setPosition(glm::vec2(0, 0));

		entity = builder.getResults();
	
		registerEntity(entity);
	}
	void SandboxSecene::onDestroy()
	{
	}
}