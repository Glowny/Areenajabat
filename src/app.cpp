#include "game_time.h"
#include "ecs\managers\sprite_manager.h"
#include "app.h"
#include <bgfx/bgfx.h>
#include "input/input.h"
#include "input/event.h"
#include <bx/bx.h>
#include <bx/crtimpl.h>
#include "res/resource_manager.h"
#include <bx/fpumath.h>
#include "io/io.h"
#include "utils/bgfx_utils.h"
#include <glm/gtc/matrix_transform.hpp>
#include "graphics/spritebatch.h"
#include "res/shader_resource.h"
#include "res/texture_resource.h"
#include <bx/timer.h>
#include "graphics/sprite_animation.h"
#include "render.h"
#include "res/spriter_resource.h"
#include "res/spriter_animation_player.h"
#include <glm/gtc/type_ptr.hpp>
#include "utils/math.h"
#include "camera.h"
#include "graphics/composite_sprite.h"
#include "scenes/scene_manager.h"
#include "scenes/sandbox_scene.h"

namespace arena
{

    static MouseState s_mouseState;
    
    struct Crosshair
    {
        Crosshair()
            : m_texture(App::instance().resources()->get<TextureResource>(ResourceType::Texture, "crosshair.png"))
        {

        }
        TextureResource* m_texture;
        glm::vec2 m_position;
    };

    struct AnimationType
    {
        enum Enum
        {
            Gladius = 0,

            Count
        };
    };

    struct RightArm
    {
        RightArm() :
            m_upperAngle(70.f),
            m_forearmAngle(40.f),
            m_gunAngle(250.f),
            m_upperArm(App::instance().resources()->get<TextureResource>(ResourceType::Texture, "Characters/arms/1_UpperArm.png")),
            m_foreArm(App::instance().resources()->get<TextureResource>(ResourceType::Texture, "Characters/arms/1_Forearm.png")),
            m_gun(App::instance().resources()->get<TextureResource>(ResourceType::Texture, "Characters/guns/GladiusLeft.png")),
            //m_laser(App::instance().resources()->get<TextureResource>(ResourceType::Texture, "blank.png")),
            m_flipX(false)
        {
            // setup children hierarchy
            m_upperArm.m_children.push_back(&m_foreArm);
            m_foreArm.m_children.push_back(&m_gun);
            //m_gun.m_children.push_back(&m_laser);

            // upper arm
            m_upperArm.m_origin = glm::vec2(m_upperArm.m_texture->width / 2.f, 10.f);
            m_upperArm.m_rotation = glm::radians(m_upperAngle);
            m_upperArm.m_depth = 2.f;

            // fore arm
            m_foreArm.m_origin = glm::vec2(m_foreArm.m_texture->width / 2.f, 10.f);;
            m_foreArm.m_position = glm::vec2(5, 30);
            m_foreArm.m_rotation = glm::radians(m_forearmAngle);
            m_foreArm.m_depth = 2.f;

            // gun
            m_gun.m_depth = 1.9f;
            m_gun.m_origin = glm::vec2(m_gun.m_texture->width, m_gun.m_texture->height) / 2.f;
            m_gun.m_rotation = glm::radians(m_gunAngle);
            m_gun.m_position = glm::vec2(5.f, 18.f); // 10.f

            /*// laser
            m_laser.m_scale = glm::vec2(400, 1);
            m_laser.m_position = glm::vec2(0, 23.f);
            m_laser.m_rotation = glm::radians(-180.f);
            m_laser.m_depth = 100.f;*/
        }

        void flip()
        {
            m_flipX = !m_flipX;

            if (m_flipX)
            {
                m_upperArm.m_rotation = glm::radians(-m_upperAngle);
                m_foreArm.m_rotation = glm::radians(-m_forearmAngle);
                m_gun.m_rotation = glm::radians(-m_gunAngle);
                m_foreArm.m_position.x = 9.f;
            }
            else
            {
                m_upperArm.m_rotation = glm::radians(m_upperAngle);
                m_foreArm.m_rotation = glm::radians(m_forearmAngle);
                m_gun.m_rotation = glm::radians(m_gunAngle);
                m_foreArm.m_position.x = 5.f;
            }
        }

        void render()
        {
            SpriteEffects::Enum effects = m_flipX ? SpriteEffects::FlipHorizontally : SpriteEffects::None;
            m_upperArm.render(effects);
        }

        // left angles
        float m_upperAngle;
        float m_forearmAngle;
        float m_gunAngle;

        CompositeSprite m_upperArm;
        CompositeSprite m_foreArm;
        CompositeSprite m_gun;
        //CompositeSprite m_laser;

        bool m_flipX;
    };

    struct Head
    {
        Head() :
            m_helmet(App::instance().resources()->get<TextureResource>(ResourceType::Texture, "Characters/head/1_Helmet.png")),
            m_crest(App::instance().resources()->get<TextureResource>(ResourceType::Texture, "Characters/head/1_Crest.png"))
        {
            // setup hierarchy
            m_helmet.m_children.push_back(&m_crest);

            m_helmet.m_position = glm::vec2(0, -28);

            m_crest.m_position = glm::vec2(0, -9);
        }

        CompositeSprite m_helmet;
        CompositeSprite m_crest;
    };

    class Character
    {
    public:
        glm::vec2 m_reloadOffset;

        Character() :
            m_legs(App::instance().resources()->get<SpriterResource>(ResourceType::Spriter, "Characters/Animations/LegAnimations/Run.scml")->getNewEntityInstance(0)),
            m_reload(App::instance().resources()->get<SpriterResource>(ResourceType::Spriter, "Characters/Animations/ReloadingAnimations/Gladius.scml")->getNewEntityInstance(0)),
            m_torso(App::instance().resources()->get<TextureResource>(ResourceType::Texture, "Characters/body/1_Torso.png")),
            m_torsoOffset(-6.f, 37.f),
            m_legOffset(11, 124),
            m_reloadOffset(12, 45),
            m_elapsed(0.0),
            m_flipX(false)
        {
            m_legs.setCurrentAnimation("1_Left");
            m_reload.setCurrentAnimation(0);
            
            // setup hierarchy, torso holds head and arms
            m_torso.m_children.push_back(&m_head.m_helmet);
            m_torso.m_children.push_back(&m_arm.m_upperArm);

            //m_torso.m_origin = glm::vec2(m_torso.m_texture->width / 2.f, m_torso.m_texture->height - 4);

            m_arm.m_upperArm.m_position = glm::vec2(16, 10);
            setPosition(glm::vec2(0.f));
        }

        void update(float dt)
        {
            static const uint32_t length = 600;

            static const float s_directionTransitionTable[] =
            {
                0.f, // 0
                -1.f, // 50
                1.f, // 100
                2.f, // 150
                1.f, // 200
                0.5f, // 250
                0.f, // 300
                -1.f, // 350
                1.f, // 400
                2.f, // 450
                1.f, // 500
                0.5f // 550
            };

            m_elapsed += dt;

            const uint32_t asMillis = uint32_t(m_elapsed * 1000.0);

            uint32_t index = uint32_t(asMillis / 50) % BX_COUNTOF(s_directionTransitionTable);
            uint32_t nextIndex = (index + 1) % BX_COUNTOF(s_directionTransitionTable);

            float t = (asMillis % 50) / 50.f;
            m_perFrameTorsoOffset.y = lerp<float>(s_directionTransitionTable[index], s_directionTransitionTable[nextIndex], t);

            m_legs.setTimeElapsed(dt * 1000.0);
            m_reload.setTimeElapsed(dt * 1000.0);

            if (asMillis >= length)
            {
                m_elapsed = m_elapsed - 0.6;
            }

            m_torso.m_position = m_position + m_torsoOffset + m_perFrameTorsoOffset;
            glm::vec2 mouseLoc(s_mouseState.m_mx, s_mouseState.m_my);
            transform(mouseLoc, glm::inverse(App::instance().camera().m_matrix), &mouseLoc);

            m_cross.m_position = mouseLoc - glm::vec2(m_cross.m_texture->width, m_cross.m_texture->height) / 2.f;

            const glm::vec2 handpos = m_arm.m_upperArm.m_globalPosition;
            glm::vec2 dir(mouseLoc - handpos);            

            m_flipX = mouseLoc.x >= m_position.x;

            float a = glm::atan(dir.y, dir.x);

            if (m_flipX)
            {
                m_arm.m_upperArm.m_rotation = glm::radians(-m_arm.m_upperAngle) + a;
            }
            else
            {
                m_arm.m_upperArm.m_rotation = glm::radians(m_arm.m_upperAngle) - glm::radians(180.f) + a;
            }

            static float angle = 0.000f;
            m_reload.setAngle(glm::radians(90.f));
        }

        void render()
        {
            SpriteEffects::Enum effects = SpriteEffects::None;
            if (m_flipX)
            {
                effects = SpriteEffects::FlipHorizontally;
                if (!m_arm.m_flipX)
                    m_arm.flip();

                if (m_legs.getCurrentAnimationName() == "1_Left")
                {
                    m_legs.setCurrentAnimation("1_Right");
                    m_reload.setCurrentAnimation("1_Right");
                    m_elapsed = m_legs.getCurrentTime();
                }

                m_torsoOffset.x = -6.f;
            }
            else
            {
                if (m_arm.m_flipX) m_arm.flip();

                if (m_legs.getCurrentAnimationName() == "1_Right")
                {
                    m_legs.setCurrentAnimation("1_Left");
                    m_reload.setCurrentAnimation("1_Left");
                    m_elapsed = m_legs.getCurrentTime();
                }

                m_torsoOffset.x = -4.f;
            }

            
            m_reload.setPosition(m_position + m_reloadOffset + m_perFrameTorsoOffset);
            m_reload.render();
            m_legs.render();
            
            m_torso.render(effects);

            draw(m_cross.m_texture, nullptr, 0xffffffff, m_cross.m_position, glm::vec2(0, 0), glm::vec2(1, 1), SpriteEffects::None, 0.f, 1.f);
        }

        void setPosition(const glm::vec2& position)
        {
            m_position = position;
            m_legs.setPosition(position + m_legOffset);
        }

    public:
        SpriterAnimationPlayer m_legs;
        SpriterAnimationPlayer m_reload;

        CompositeSprite m_torso;
        RightArm m_arm;
        Head m_head;

        Crosshair m_cross;

        double m_elapsed;

        // global position
        glm::vec2 m_position;

        // relative from position
        glm::vec2 m_legOffset;
        // relative from position
        glm::vec2 m_torsoOffset;

        glm::vec2 m_perFrameTorsoOffset;

        bool m_flipX;
    };

    static Character* s_char;
	/*
		Static members.
	*/

    static void cmdExit(const void*);
    static void moveLeft(const void*);
    static void moveRight(const void*);
    static void moveDown(const void*);
    static void moveUp(const void*);

	static const InputBinding		s_bindings[] =
	{
		{ arena::Key::KeyQ, arena::Modifier::LeftCtrl, 0, cmdExit, "exit" },
        { arena::Key::KeyW, arena::Modifier::None, 0, moveUp, "move up" },
        { arena::Key::KeyA, arena::Modifier::None, 0, moveLeft, "move up" },
        { arena::Key::KeyS, arena::Modifier::None, 0, moveDown, "move up" },
        { arena::Key::KeyD, arena::Modifier::None, 0, moveRight, "move up" },
		INPUT_BINDING_END
	};

	static double		s_timeSinceStart = 0;
	static int64_t		s_last_time = 0;
	static bool			s_exit = false;
	static uint32_t		s_reset = BGFX_RESET_NONE;

	/*
		Static and non-member functions.	
	*/
    
	static void cmdExit(const void*)
    {
        s_exit = true;
    }

	static bool processEvents(int32_t& width, int32_t& height)
	{
		const Event* ev;

		uint32_t reset = s_reset;

		do {
			struct SE { const Event* m_ev; SE() : m_ev(poll()) {} ~SE() { if (NULL != m_ev) { release(m_ev); } } } scopeEvent;
			ev = scopeEvent.m_ev;

			SDL_Window* window = nullptr;

			if (NULL != ev)
			{
				switch (ev->m_type)
				{
				case Event::Char:
				{
					const CharEvent* chev = static_cast<const CharEvent*>(ev);
					inputChar(chev->m_len, chev->m_char);
				}
				break;
				case Event::Exit:
					return true;
				case Event::Mouse:
				{
					const MouseEvent* mouse = static_cast<const MouseEvent*>(ev);
					window = mouse->m_window;

					if (mouse->m_move)
					{
						inputSetMousePos(mouse->m_mx, mouse->m_my, mouse->m_mz);
						s_mouseState.m_mx = mouse->m_mx;
						s_mouseState.m_my = mouse->m_my;
						s_mouseState.m_mz = mouse->m_mz;
					}
					else
					{
						inputSetMouseButtonState(mouse->m_button, mouse->m_down);
						s_mouseState.m_buttons[mouse->m_button] = mouse->m_down;
					}
				}
				break;
				case Event::Key:
				{
					const KeyEvent* key = static_cast<const KeyEvent*>(ev);
					window = key->m_window;

					inputSetKeyState(key->m_key, key->m_modifiers, key->m_down);
				}
				break;

				case Event::Size:
				{
					const SizeEvent* size = static_cast<const SizeEvent*>(ev);
					window = size->m_window;
					width = size->m_width;
					height = size->m_height;
					reset = !s_reset; // force reset
				}
				break;
				default:
					break;
				}
			}

			inputProcess();

		} while (ev != NULL);

		if (reset != s_reset)
		{
			bgfx::reset(width, height, s_reset);
			inputSetMouseResolution(uint16_t(width), uint16_t(height));
		}

		// return true if we want to exit
		return false;
	}

	bx::AllocatorI* getAllocator()
	{
		static bx::CrtAllocator s_allocator;
		return &s_allocator;
	}

	void draw(const TextureResource* texture, glm::vec4* src, uint32_t color, 
        const glm::vec2& position, const glm::vec2& origin, const glm::vec2& scale, 
        uint8_t effect, float angle, float depth)
	{
		App::instance().spriteBatch()->draw(
			texture,
			src,
			color,
			position,
			origin,
			scale,
            effect,
			angle,
			depth
		);
	}

	/*
		App member functions.
	*/

	App::App()
	{
	}

	App& App::instance()
	{
		static App app;

		return app;
	}

    void App::init(int32_t width, int32_t height)
    {
		m_width = width;
		m_height = height;
		m_camera = Camera(float32(width), float32(height));

        s_last_time = bx::getHPCounter();

        char workingdir[512];


#if _WIN32 || _WIN64
        // TODODODODODO change dir to folder root
        bx::chdir("..\\..\\..\\");
        bx::pwd(workingdir, 512);
        printf("CWD: %s\n", workingdir);
#endif
        inputAddBindings("bindings", s_bindings);
        

        bgfx::reset(width, height, s_reset);
        bgfx::setDebug(BGFX_DEBUG_TEXT);

        // Set view 0 clear state.
        bgfx::setViewClear(0
            , BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH
            , 0x303030ff
            , 1.0f
            , 0
            );

        m_resources = new ResourceManager("assets/");
        m_spriteBatch = new SpriteBatch;
        
        s_char = new Character;

		SandboxSecene* scene = new  SandboxSecene();
		
		SceneManager::instance().push(scene);
		
		scene->activate();

        s_char->setPosition(glm::vec2(1280.f, 0.f));
    }

	bool App::update()
	{
		// return true if we want to exit
		if (processEvents(m_width, m_height)) return true;

		int64_t currentTime = bx::getHPCounter();
		const int64_t time = currentTime - s_last_time;
		s_last_time = currentTime;

		const double frequency = (double)bx::getHPFrequency();

		// seconds
		float lastDeltaTime = float(time * (1.0 / frequency));
		s_timeSinceStart += lastDeltaTime;
		GameTime gameTime(lastDeltaTime, s_timeSinceStart);

        bgfx::touch(0);
        
        m_camera.m_position = s_char->m_position;
        m_camera.m_zoom = 1.f;
		m_camera.calculate();


		float ortho[16];
		bx::mtxOrtho(ortho, 0.0f, float(m_width), float(m_height), 0.0f, 0.0f, 1000.0f);
		bgfx::setViewTransform(0, glm::value_ptr(m_camera.m_matrix), ortho);
		bgfx::setViewRect(0, 0, 0, uint16_t(m_width), uint16_t(m_height));

		bgfx::touch(0);
		bgfx::dbgTextClear();
		bgfx::dbgTextPrintf(0, 1, 0x4f, "Perkeleen perkele");
		bgfx::dbgTextPrintf(0, 2, 0x6f, "Mouse x = %d, y = %d, wheel = %d", s_mouseState.m_mx, s_mouseState.m_my, s_mouseState.m_mz);
		bgfx::dbgTextPrintf(0, 3, 0x8f, "Left btn = %s, Middle btn = %s, Right btn = %s",
			s_mouseState.m_buttons[MouseButton::Left] ? "down" : "up",
			s_mouseState.m_buttons[MouseButton::Middle] ? "down" : "up",
			s_mouseState.m_buttons[MouseButton::Right] ? "down" : "up");
		bgfx::dbgTextPrintf(0, 4, 0x9f, "Delta time %.10f", lastDeltaTime);

        
        s_char->update(lastDeltaTime);

        glm::vec2 mouseLoc(s_mouseState.m_mx, s_mouseState.m_my);
        transform(mouseLoc, glm::inverse(m_camera.m_matrix), &mouseLoc);

        bgfx::dbgTextPrintf(0, 5, 0x9f, "Delta time x= %.2f, y=%.2f", mouseLoc.x, mouseLoc.y);

        s_char->render();

		m_spriteBatch->submit(0);

		// Update systems.
		SceneManager::instance().update(gameTime);
		SpriteManager::instance().update(gameTime);

		bgfx::frame();

		return s_exit;
	}

	void App::shutdown()
	{
		inputRemoveBindings("bindings");
        inputShutdown();

		delete m_resources;
		m_resources = NULL;
		delete m_spriteBatch;
		m_spriteBatch = NULL;
	}

	SpriteBatch* const App::spriteBatch()
	{
		return m_spriteBatch;
	}
	ResourceManager* const App::resources()
	{
		return m_resources;
	}
	Camera& App::camera()
	{
		return m_camera;
	}

    static void moveLeft(const void*)
    {
        s_char->setPosition(s_char->m_position + glm::vec2(-0.1f, 0.f));
    }
    static void moveRight(const void*)
    {
        s_char->setPosition(s_char->m_position + glm::vec2(0.1f, 0.f));
    }
    static void moveDown(const void*)
    {
        s_char->setPosition(s_char->m_position + glm::vec2(-0.0f, 0.1f));
    }
    static void moveUp(const void*)
    {
        s_char->setPosition(s_char->m_position + glm::vec2(-0.0f, -0.1f));
    }
}