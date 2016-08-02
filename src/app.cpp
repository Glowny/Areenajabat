#include <common/network_interface.h>
#include "game_time.h"
#include "ecs/managers/sprite_manager.h"
#include "app.h"
#include <bgfx/bgfx.h>
#include "input/input.h"
#include "input/event.h"
#include <bx/bx.h>
#include <bx/crtimpl.h>
#include "res/resource_manager.h"
#include <bx/fpumath.h>
#include "io/io.h"
#include "graphics/spritebatch.h"
#include "res/texture_resource.h"
#include <bx/timer.h>
#include "render.h"
#include "res/spriter_resource.h"
#include <glm/gtc/type_ptr.hpp>
#include "camera.h"
#include "graphics/composite_sprite.h"
#include "scenes/scene_manager.h"
#include "graphics/layers.h"
#include "scenes/sandbox_scene.h"
#include "graphics/character_animator.h"
#include "utils/math.h"
#include <common/debug.h>

namespace arena
{

    static MouseState s_mouseState;

    const MouseState& Mouse::getState()
    {
        return s_mouseState;
    }

	static double		s_timeSinceStart = 0;
	static int64_t		s_last_time = 0;
	static bool			s_exit = false;
	static uint32_t		s_reset = BGFX_RESET_NONE;

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
                    s_exit = true;
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
        bool result = networkInitialize(); (void)result;
        ARENA_ASSERT(result == true, "Failed to initialize network");

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
        
        animationSystemInit();

        m_spriteBatch = new SpriteBatch;
       
		SandboxScene* scene = new SandboxScene();
		
		SceneManager::instance().push(scene);
		
		scene->activate();
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

        // clear screen if there is no other draw calls
        bgfx::touch(0);
				
        // Update systems.
        SceneManager::instance().update(gameTime);

		bgfx::frame();

		return s_exit;
	}

	void App::shutdown()
	{   
        networkShutdown();

        SceneManager::instance().pop()->destroy();

        animationSystemShutdown();

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
}