#define ARENA_CLIENT
#include "network-common/GamePackets.h"
#include "NetworkClient.h"

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

#include <bx/thread.h>
#include <queue>


namespace arena
{
    bx::Mutex receiveMutex;
    bx::Mutex sendMutex;

    std::queue<Message> sendQueue;
    std::queue<unsigned char*> messageQueue;
    unsigned playerID;
    bx::Thread networkThred;
    static bool s_exit = false;
    static int32_t networkProc(void*)
    {
        Network net;
        net.setMessageQueue(&messageQueue);
        net.setSendQueue(&sendQueue);
        net.connectServer("localhost", 8888);
        net.setMutex(&receiveMutex, &sendMutex);
        while (!s_exit)
        {
            net.checkEvent();
        }
        return EXIT_SUCCESS;
    }


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

    static const InputBinding s_bindings[] =
    {
        { arena::Key::KeyA, arena::Modifier::None, 0, left, "left" },
        { arena::Key::KeyD, arena::Modifier::None, 0, right, "right" },
        INPUT_BINDING_END
    };

	SandboxSecene::SandboxSecene() : Scene("sandbox")
	{
	}

	void SandboxSecene::onUpdate(const GameTime& gameTime)
	{
        while(messageQueue.size() != 0)
        { 
            unsigned char* message = getFrontMessage();
            MessageIdentifier id = getMessageID(message);
            switch (id)
            {
                case Start:
                {
                    unsigned playerAmount;
                    openSetupPacket(message, playerAmount, playerID);
                }
                case Update:
                {
                
                }
                case PlatformData:
                {

                }
       
                case CreateBullet:
                {

                }
                default:
                    break;
            }
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
        networkThred.init(networkProc);

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
        s_exit = true;
        networkThred.shutdown();
	}

    unsigned char* SandboxSecene::getFrontMessage()
    {
        // IF causes issues, keep locked till message is deleted.
        receiveMutex.lock();
        unsigned char* message = messageQueue.front();
        receiveMutex.unlock();
        return message;
    }

    void SandboxSecene::deleteFrontMessage()
    {
        receiveMutex.lock();
        messageQueue.pop();
        receiveMutex.unlock();
    }

    void SandboxSecene::setBackMessage(Message message)
    {
        sendMutex.lock();
        sendQueue.push(message);
        sendMutex.unlock();
    }

}