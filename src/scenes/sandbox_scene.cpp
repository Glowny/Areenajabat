#include "../net/network_client.h"
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
#include "../input/event.h"
#include "../utils/math.h"
#include <glm/gtc/matrix_inverse.hpp>
#include <common/packet.h>

namespace arena
{
    static double s_stamp = 0.0;
    static NetworkClient* s_client;

	static Entity* entity;
    static Animator* s_animator;

    static void left(const void*)
    {
        s_animator->m_animator.setFlipX(false);
    }

    static void right(const void*)
    {
        s_animator->m_animator.setFlipX(true);
    }
    
    static void connect(const void*)
    {
        if (s_client->isConnected()) return;
        if (s_client->isConnecting()) return;

        s_client->connect("localhost", uint16_t(8088), s_stamp);
        s_client->createLobby("perkele", s_stamp);
    }

    static void disconnect(const void*)
    {
        if (!s_client->isConnected()) return;

        s_client->disconnect(s_stamp);
    }

    static const InputBinding s_bindings[] =
    {
        { arena::Key::KeyA, arena::Modifier::None, 0, left, "left" },
        { arena::Key::KeyD, arena::Modifier::None, 0, right, "right" },
        { arena::Key::KeyQ, arena::Modifier::None, 0, connect, "connect" },
        { arena::Key::KeyE, arena::Modifier::None, 0, disconnect, "disconnect" },
        INPUT_BINDING_END
    };

	SandboxSecene::SandboxSecene() : Scene("sandbox")
	{
	}

	void SandboxSecene::onUpdate(const GameTime& gameTime)
	{
        s_stamp = gameTime.m_total;

        s_client->sendMatchMakingPackets(gameTime.m_total);
        s_client->sendProtocolPackets(gameTime.m_total);

        s_client->writePackets();
        s_client->readPackets();

        Packet* packet = nullptr;
        ENetPeer* from;
        while ((packet = s_client->receivePacket(from)) != nullptr)
        {
            // "protocol" messages
            if (packet->getType() <= PacketTypes::Disconnect)
            {
                s_client->processClientSidePackets(packet, from, gameTime.m_total);
            }
            else
            {

            }

            destroyPacket(packet);
        }
        auto tx = (Transform* const)entity->first(TYPEOF(Transform));

        Camera& camera = App::instance().camera();
        camera.m_position = tx->m_position;
        camera.calculate();
        // set views
        float ortho[16];
        bx::mtxOrtho(ortho, 0.0f, float(camera.m_bounds.x), float(camera.m_bounds.y), 0.0f, 0.0f, 1000.0f);
        bgfx::setViewTransform(0, glm::value_ptr(camera.m_matrix), ortho);
        bgfx::setViewRect(0, 0, 0, uint16_t(camera.m_bounds.x), uint16_t(camera.m_bounds.y));

        bgfx::dbgTextClear();

        SpriteManager::instance().update(gameTime);
        AnimatorManager::instance().update(gameTime);
        
        const MouseState& mouse = Mouse::getState();

        glm::vec2 mouseLoc(mouse.m_mx, mouse.m_my);
        transform(mouseLoc, glm::inverse(camera.m_matrix), &mouseLoc);

        glm::vec2 dir(mouseLoc - tx->m_position);
        float a = glm::atan(dir.y, dir.x);

        bgfx::dbgTextPrintf(0, 1, 0x9f, "Delta time %.10f", gameTime.m_delta);
        bgfx::dbgTextPrintf(0, 2, 0x8f, "Left btn = %s, Middle btn = %s, Right btn = %s",
            mouse.m_buttons[MouseButton::Left] ? "down" : "up",
            mouse.m_buttons[MouseButton::Middle] ? "down" : "up",
            mouse.m_buttons[MouseButton::Right] ? "down" : "up");
        bgfx::dbgTextPrintf(0, 3, 0x6f, "Mouse (screen) x = %d, y = %d, wheel = %d", mouse.m_mx, mouse.m_my, mouse.m_mz);
        bgfx::dbgTextPrintf(0, 4, 0x9f, "Mouse pos (world) x= %.2f, y=%.2f", mouseLoc.x, mouseLoc.y);
        bgfx::dbgTextPrintf(0, 5, 0x9f, "Angle (%.3f rad) (%.2f deg)", a, glm::degrees(a));

        App::instance().spriteBatch()->submit(0);
	}

	void SandboxSecene::onInitialize()
	{
        s_client = new NetworkClient(uint16_t(13337));
		EntityBuilder builder;

		builder.begin();
		
        ResourceManager* resources = App::instance().resources();

		SpriteRenderer* renderer = builder.addSpriteRenderer();
		renderer->setTexture(resources->get<TextureResource>(ResourceType::Texture, "perkele.png"));
		renderer->anchor();

		Transform* transform = builder.addTransformComponent();
		transform->m_position.x = 500.f;
		transform->m_position.y = 500.0f;


        s_animator = builder.addCharacterAnimator();
        CharacterAnimator& anim = s_animator->m_animator;
        anim.setStaticContent(
            resources->get<TextureResource>(ResourceType::Texture, "Characters/head/1_Crest.png"),
            resources->get<TextureResource>(ResourceType::Texture, "Characters/head/1_Helmet.png"),
            resources->get<TextureResource>(ResourceType::Texture, "Characters/body/1_Torso.png"),
            resources->get<SpriterResource>(ResourceType::Spriter, "Characters/Animations/LegAnimations/Run.scml")->getNewEntityInstance(0)
        );
        anim.setWeaponAnimation(WeaponAnimationType::Gladius);
        //anim.setPosition(glm::vec2(0, 0));

		entity = builder.getResults();
	
		registerEntity(entity);

        inputAddBindings("player", s_bindings);
	}
	void SandboxSecene::onDestroy()
	{
        inputRemoveBindings("player");
	}
}