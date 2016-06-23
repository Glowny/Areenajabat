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

namespace arena
{
    static bool s_exit = false;
    static uint32_t s_reset = BGFX_RESET_NONE;

    static MouseState s_mouseState;

    static ResourceManager* s_resources;

    static SpriteBatch* s_spriteBatch;

    static void cmdExit(const void*)
    {
        s_exit = true;
    }


    struct Character
    {
        Character()
            : m_legs(
                getResources()->get<TextureResource>(
                    ResourceType::Texture,
                    "character/Run_BareLegs1_ss.png"),
                64, 64, 1024 / 64),

            m_greaves(
                getResources()->get<TextureResource>(
                    ResourceType::Texture,
                    "character/Run_Greaves1_ss.png"),
                64, 64, 1024 / 64),
            m_torso(
                getResources()->get<TextureResource>(
                    ResourceType::Texture,
                    "character/Torso1.png")
            ),
            m_torsoPosition(glm::vec2(117, 280)),
            m_elapsed(0)
        {

        }

        void update(float dt)
        {
            m_elapsed += dt;
            //printf("%.5f\n", m_elapsed);
            m_torsoPosition.y += cosf(m_elapsed * 6.f) * 0.5f;
            m_legs.update(dt);
            m_greaves.update(dt);
        }

        float m_elapsed;
        // draw this first
        SpriteAnimation m_legs;
        // draw this then
        SpriteAnimation m_greaves;

        TextureResource* m_torso;

        glm::vec2 m_torsoPosition;
    };

    static Character* s_char;

    static const InputBinding s_bindings[] =
    {
        { arena::Key::KeyQ, arena::Modifier::LeftCtrl, 0, cmdExit, "exit" },
        INPUT_BINDING_END
    };

    struct Camera
    {
        glm::vec2 m_position;
        float m_zoom;
        float m_angle;
        glm::vec2 m_bounds;
        glm::mat4 m_matrix;

        Camera(float width, float height)
            : 
            m_position(0.f, 0.f),
            m_zoom(1.f),
            m_angle(0.f),
            m_bounds(width / 2.f, height / 2.f),
            m_matrix(1.f)
        {

        }

        void calculate()
        {
            m_matrix =
                glm::translate(glm::mat4(1.f), glm::vec3(m_position, 0)) *
                glm::rotate(glm::mat4(1.f), m_angle, glm::vec3(0, 0, 1.f)) *
                glm::scale(glm::mat4(1.f), glm::vec3(m_zoom)) *
                glm::translate(glm::mat4(1.f), glm::vec3(m_bounds, 0.f));

        }
    };

    uint32_t toABGR(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = 255)
    {
        return (alpha << 24) | (blue << 16) | (green << 8) | red;
    }

    uint32_t toABGR(uint32_t rgbaHex)
    {
        return
            (((rgbaHex >> 0) & 0xFF) << 24) | // alpha
            (((rgbaHex >> 8) & 0xFF) << 16) | // blue
            (((rgbaHex >> 16) & 0xFF) << 8) | // green
            (((rgbaHex >> 24) & 0xFF) << 0);   // red
    }

    static Camera s_camera(1280.f, 720.f);

    static int64_t s_last_time = 0;

    static SpriteAnimation* s_animation;

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
        inputInit();
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

        s_animation = new SpriteAnimation(
            getResources()->get<TextureResource>(ResourceType::Texture, "juoksu_ss.png"),
            uint16_t(64),
            uint16_t(64),
            3 * 4 + 2
        );

        s_char = new Character;
    }

    // return trues if we want to exit
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

        return false;
    }

    // return true if exit
    bool App::update()
    {
        if (processEvents(width, height)) return true;

        int64_t currentTime = bx::getHPCounter();
        const int64_t time = currentTime - s_last_time;
        s_last_time = currentTime;
        
        const double frequency = (double)bx::getHPFrequency();

        // seconds
        float lastDeltaTime = float(time * (1.0 / frequency));

        s_camera.calculate();

        float ortho[16];
        bx::mtxOrtho(ortho, 0.0f, float(width), float(height), 0.0f, 0.0f, 1000.0f);
        bgfx::setViewTransform(0, /*glm::value_ptr(s_camera.m_matrix)*/NULL, ortho);
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

        //s_sprite.m_origin = glm::vec2(s_sprite.m_res->width / 2.f, s_sprite.m_res->height / 2.f);
     //   auto tex = getResources()->get<TextureResource>(ResourceType::Texture, "juoksu_ss.png");
      //  auto tex2 = getResources()->get<TextureResource>(ResourceType::Texture, "rgb.png");


        s_animation->update(lastDeltaTime);
        s_char->update(lastDeltaTime);
        
        static float angle = 0.001f;
        angle += 0.001f;
        
        //s_spriteBatch->draw(tex, &src, 0xFFFFFFFF, glm::vec2(400, 300), /*glm::vec2(tex->width / 2.f, tex->height / 2.f)*/glm::vec2(32,32), glm::vec2(1.0, 1.0), angle);
        //s_spriteBatch->draw(tex, 0xFFFFFFFF, glm::vec2(300, 300));
        //s_spriteBatch->draw(tex2, 0xFFFFFFFF, glm::vec2(0, 0));

        auto i = s_char->m_legs.m_currentFrame;
        Frame& frame = s_char->m_legs.m_frames[i];
        glm::vec4 src(frame.x, frame.y, s_char->m_legs.m_frameWidth, s_char->m_legs.m_frameHeight);
        s_spriteBatch->draw(s_char->m_torso, nullptr, 0xffffffff, s_char->m_torsoPosition, glm::vec2(0,0), glm::vec2(1,1), 0.f, 0.f);
        s_spriteBatch->draw(s_char->m_legs.m_spritesheet, &src, 0xffffffff, glm::vec2(100, 300), glm::vec2(0, 0), glm::vec2(1, 1), 0.f, 1.f);
        s_spriteBatch->draw(s_char->m_greaves.m_spritesheet, &src, 0xffffffff, glm::vec2(100, 300), glm::vec2(0, 0), glm::vec2(1, 1), 0.f, 2.f);
        s_spriteBatch->submit(0);

        bgfx::frame();

        //bx::sleep(16);

        return s_exit;
    }

    void App::shutdown()
    {
        inputRemoveBindings("bindings");
        inputShutdown();

        delete s_resources;
        s_resources = NULL;
        delete s_spriteBatch;
        s_spriteBatch = NULL;
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
}