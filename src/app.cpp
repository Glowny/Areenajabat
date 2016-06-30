#include "game_time.h"
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

    struct Crosshair
    {
        Crosshair()
            : m_texture(getResources()->get<TextureResource>(ResourceType::Texture, "crosshair.png"))
        {

        }
        TextureResource* m_texture;
        glm::vec2 m_position;
    };

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

    class Character
    {
    public:
        Character()
            : m_legs(getResources()->get<SpriterResource>(ResourceType::Spriter, "player/legs.scml")->getNewEntityInstance(0)),
            m_helmet(getResources()->get<TextureResource>(ResourceType::Texture, "player/head/Helmet.png")),
            m_torso(getResources()->get<TextureResource>(ResourceType::Texture, "player/body/Torso.png")),
            m_crest(getResources()->get<TextureResource>(ResourceType::Texture, "player/head/Crest4.png")),
            m_rightUpperArm(getResources()->get<TextureResource>(ResourceType::Texture, "player/arms/RightUpperArm.png")),
            m_rightForeArm(getResources()->get<TextureResource>(ResourceType::Texture, "player/arms/RightForearm.png")),
            m_gunTexture(getResources()->get<TextureResource>(ResourceType::Texture, "player/gun/GladiusLeft.png")),
            m_crestOffset(-1.5f, 0.f),
            m_helmetOffset(-3.f, 14.f),
            m_torsoOffset(0.f, 37.f),
            m_legOffset(11, 124),
            m_elapsed(0.0),
            m_rightUpperArmOffset(11.f, 46.f),
            m_rightArmSprite(m_rightUpperArm),
            m_rightForeArmSprite(m_rightForeArm),
            m_gunSprite(m_gunTexture)
            
        {
            m_legs.setCurrentAnimation(0);

            m_rightArmSprite.m_children.push_back(&m_rightForeArmSprite);
            m_rightArmSprite.m_origin = glm::vec2(m_rightUpperArm->width / 2.f, 5.f);
            m_rightArmSprite.m_rotation = glm::radians(70.f);
            m_rightArmSprite.m_depth = 2.f;

            m_rightForeArmSprite.m_origin = glm::vec2(m_rightForeArm->width / 2.f, m_rightUpperArm->height + 5.f);;
            m_rightForeArmSprite.m_position = glm::vec2(-12, 45);
            m_rightForeArmSprite.m_rotation = glm::radians(40.f);
            m_rightForeArmSprite.m_depth = 2.f;

            m_rightForeArmSprite.m_children.push_back(&m_gunSprite);
            m_gunSprite.m_depth = 1.9f;
            m_gunSprite.m_origin = glm::vec2(m_gunTexture->width, m_gunTexture->height) / 2.f;
            m_gunSprite.m_rotation = glm::radians(250.f);
            m_gunSprite.m_position = glm::vec2(05.f, 15.f);

            setPosition(glm::vec2(0.f));
        }

        void update(float dt)
        {
            static const uint32_t length = 600;

            m_elapsed += dt;

            const uint32_t asMillis = uint32_t(m_elapsed * 1000.0);

            uint32_t index = uint32_t(asMillis / 50) % BX_COUNTOF(s_directionTransitionTable);
            uint32_t nextIndex = (index + 1) % BX_COUNTOF(s_directionTransitionTable);

            float t = (asMillis % 50) / 50.f;
            m_perFrameTorsoOffset.y = lerp<float>(s_directionTransitionTable[index], s_directionTransitionTable[nextIndex], t);

            m_legs.setTimeElapsed(dt * 1000.0);

            if (asMillis >= length)
            {
                m_elapsed = m_elapsed - 0.6;
            }

            m_rightArmSprite.m_position = m_position + m_rightUpperArmOffset + m_perFrameTorsoOffset;
        }

        void render()
        {
            m_legs.render();
            draw(m_crest, nullptr, 0xffffffff, m_position + m_crestOffset + m_perFrameTorsoOffset, glm::vec2(0), glm::vec2(1), 0.f, 0.f);
            draw(m_helmet, nullptr, 0xffffffff, m_position + m_helmetOffset + m_perFrameTorsoOffset, glm::vec2(0), glm::vec2(1), 0.f, 0.1f);
            draw(m_torso, nullptr, 0xffffffff, m_position + m_torsoOffset + m_perFrameTorsoOffset, glm::vec2(0), glm::vec2(1), 0.f, 0.2f);
            m_rightArmSprite.render();
        }

        void setPosition(const glm::vec2& position)
        {
            m_position = position;
            m_legs.setPosition(position + m_legOffset);
        }

    public:
        SpriterAnimationPlayer m_legs;
        TextureResource* m_helmet;
        TextureResource* m_crest;
        TextureResource* m_torso;
        TextureResource* m_gunTexture;

        TextureResource* m_rightUpperArm;
        glm::vec2 m_rightUpperArmOffset;
        double m_elapsed;

        TextureResource* m_rightForeArm;

        glm::vec2 m_position;

        glm::vec2 m_crestOffset;
        // relative from posiiton
        glm::vec2 m_helmetOffset;
        // relative from posiiton
        glm::vec2 m_legOffset;
        // relative from posiiton
        glm::vec2 m_torsoOffset;

        glm::vec2 m_perFrameTorsoOffset;

        CompositeSprite m_rightArmSprite;
        CompositeSprite m_rightForeArmSprite;
        CompositeSprite m_gunSprite;
        Crosshair m_cross;
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

    static MouseState				s_mouseState;
    static ResourceManager*			s_resources;
    static SpriteBatch*				s_spriteBatch;
	static Camera					s_camera(1280.f, 720.f);

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

	ResourceManager* getResources()
	{
		return s_resources;
	}

	void draw(const TextureResource* texture, glm::vec4* src, uint32_t color, const glm::vec2& position, const glm::vec2& origin, const glm::vec2& scale, float angle, float depth)
	{
		s_spriteBatch->draw(
			texture,
			src,
			color,
			position,
			origin,
			scale,
			angle,
			depth
		);
	}

	/*
		App member functions.
	*/

    void App::init(int32_t width, int32_t height)
    {
        this->width = width;
        this->height = height;

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

        s_resources = new ResourceManager("assets/");
        s_spriteBatch = new SpriteBatch;
        
        s_char = new Character;

		SandboxSecene* scene = new  SandboxSecene();
		
		SceneManager::instance().push(scene);
		
		scene->activate();

        s_char->setPosition(glm::vec2(1280.f, 0.f));
    }

	bool App::update()
	{
		// return true if we want to exit
		if (processEvents(width, height)) return true;

		int64_t currentTime = bx::getHPCounter();
		const int64_t time = currentTime - s_last_time;
		s_last_time = currentTime;

		const double frequency = (double)bx::getHPFrequency();

		// seconds
		float lastDeltaTime = float(time * (1.0 / frequency));
		s_timeSinceStart += lastDeltaTime;
		GameTime gameTime(lastDeltaTime, s_timeSinceStart);

        bgfx::touch(0);
        
        s_camera.m_position = s_char->m_position;
		s_camera.calculate();


		float ortho[16];
		bx::mtxOrtho(ortho, 0.0f, float(width), float(height), 0.0f, 0.0f, 1000.0f);
		bgfx::setViewTransform(0, glm::value_ptr(s_camera.m_matrix), ortho);
		bgfx::setViewRect(0, 0, 0, uint16_t(width), uint16_t(height));

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
        transform(mouseLoc, glm::inverse(s_camera.m_matrix), &mouseLoc);

        bgfx::dbgTextPrintf(0, 5, 0x9f, "Delta time x= %.2f, y=%.2f", mouseLoc.x, mouseLoc.y);

        Crosshair& cross = s_char->m_cross;
        s_char->m_cross.m_position = mouseLoc - glm::vec2(cross.m_texture->width, cross.m_texture->height) / 2.f;
        
        const glm::vec2& crosspos = s_char->m_cross.m_position;
        const glm::vec2 handpos = s_char->m_rightArmSprite.m_position + s_char->m_gunSprite.m_position;
        glm::vec2 dir(crosspos - handpos);
        
        float a = glm::atan(dir.y, dir.x) - glm::radians(180.f);
        s_char->m_rightArmSprite.m_rotation = glm::radians(65.f) + a;

        s_spriteBatch->draw(s_char->m_cross.m_texture, nullptr, 0xffffffff, s_char->m_cross.m_position, glm::vec2(0, 0), glm::vec2(1, 1), 0.f, 1.f);

        s_char->render();

		s_spriteBatch->submit(0);

		SceneManager::instance().update(gameTime);

		bgfx::frame();

		return s_exit;
	}

	void App::shutdown()
	{
		inputRemoveBindings("bindings");

		delete s_resources;
		s_resources = NULL;
		delete s_spriteBatch;
		s_spriteBatch = NULL;
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