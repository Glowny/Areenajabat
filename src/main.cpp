#define DEFAULT_WIDTH 1280
#define DEFAULT_HEIGHT 720
#include <bx/platform.h>
#if BX_PLATFORM_WINDOWS
#	define SDL_MAIN_HANDLED
#endif

#include <bx/bx.h>
#if BX_PLATFORM_LINUX
#   include </usr/include/SDL2/SDL.h>
#else
#   include <SDL2/SDL.h>
#endif

BX_PRAGMA_DIAGNOSTIC_PUSH_CLANG()
BX_PRAGMA_DIAGNOSTIC_IGNORED_CLANG("-Wextern-c-compat")
#if BX_PLATFORM_LINUX
#   include </usr/include/SDL2/SDL_syswm.h>
#else
#   include <SDL2/SDL_syswm.h>
#endif
BX_PRAGMA_DIAGNOSTIC_POP_CLANG()

#include <bgfx/bgfx.h>
#include <string.h>
#include "app.h"
#include <bx/thread.h>
#include <bgfx/bgfxplatform.h>
#if defined(None) // X11 defines this...
#	undef None
#endif // defined(None)

#include "input/input.h"
#include "input/event.h"

namespace arena
{
    int32_t thread_proc(void*);

    static uint8_t translateKeyModifiers(uint16_t _sdl)
    {
        uint8_t modifiers = 0;
        modifiers |= _sdl & KMOD_LALT ? Modifier::LeftAlt : 0;
        modifiers |= _sdl & KMOD_RALT ? Modifier::RightAlt : 0;
        modifiers |= _sdl & KMOD_LCTRL ? Modifier::LeftCtrl : 0;
        modifiers |= _sdl & KMOD_RCTRL ? Modifier::RightCtrl : 0;
        modifiers |= _sdl & KMOD_LSHIFT ? Modifier::LeftShift : 0;
        modifiers |= _sdl & KMOD_RSHIFT ? Modifier::RightShift : 0;
        modifiers |= _sdl & KMOD_LGUI ? Modifier::LeftMeta : 0;
        modifiers |= _sdl & KMOD_RGUI ? Modifier::RightMeta : 0;
        return modifiers;
    }

    static uint8_t translateKeyModifierPress(uint16_t _key)
    {
        uint8_t modifier;
        switch (_key)
        {
        case SDL_SCANCODE_LALT: { modifier = Modifier::LeftAlt;    } break;
        case SDL_SCANCODE_RALT: { modifier = Modifier::RightAlt;   } break;
        case SDL_SCANCODE_LCTRL: { modifier = Modifier::LeftCtrl;   } break;
        case SDL_SCANCODE_RCTRL: { modifier = Modifier::RightCtrl;  } break;
        case SDL_SCANCODE_LSHIFT: { modifier = Modifier::LeftShift;  } break;
        case SDL_SCANCODE_RSHIFT: { modifier = Modifier::RightShift; } break;
        case SDL_SCANCODE_LGUI: { modifier = Modifier::LeftMeta;   } break;
        case SDL_SCANCODE_RGUI: { modifier = Modifier::RightMeta;  } break;
        default: { modifier = 0;                    } break;
        }

        return modifier;
    }

    static uint8_t s_translateKey[256];

    static void initTranslateKey(uint16_t _sdl, Key::Enum _key)
    {
        BX_CHECK(_sdl < BX_COUNTOF(s_translateKey), "Out of bounds %d.", _sdl);
        s_translateKey[_sdl & 0xff] = (uint8_t)_key;
    }

    static Key::Enum translateKey(SDL_Scancode _sdl)
    {
        return (Key::Enum)s_translateKey[_sdl & 0xff];
    }

    static uint8_t s_translateGamepad[256];

    static void initTranslateGamepad(uint8_t _sdl, Key::Enum _button)
    {
        s_translateGamepad[_sdl] = _button;
    }

/*    static Key::Enum translateGamepad(uint8_t _sdl)
    {
        return Key::Enum(s_translateGamepad[_sdl]);
    }*/

    static uint8_t s_translateGamepadAxis[256];

    static void initTranslateGamepadAxis(uint8_t _sdl, GamepadAxis::Enum _axis)
    {
        s_translateGamepadAxis[_sdl] = uint8_t(_axis);
    }

/*    static GamepadAxis::Enum translateGamepadAxis(uint8_t _sdl)
    {
        return GamepadAxis::Enum(s_translateGamepadAxis[_sdl]);
    }*/

    struct Context
    {
        Context()
            : m_mx(0), m_my(0), m_mz(0), m_window(NULL), m_width(1920), m_height(900)
			//											 m_width(1920), m_height(1080)
			//											 m_width(1280), m_height(720)
        {
            memset(s_translateKey, 0, sizeof(s_translateKey));
            initTranslateKey(SDL_SCANCODE_ESCAPE, Key::Esc);
            initTranslateKey(SDL_SCANCODE_RETURN, Key::Return);
            initTranslateKey(SDL_SCANCODE_TAB, Key::Tab);
            initTranslateKey(SDL_SCANCODE_BACKSPACE, Key::Backspace);
            initTranslateKey(SDL_SCANCODE_SPACE, Key::Space);
            initTranslateKey(SDL_SCANCODE_UP, Key::Up);
            initTranslateKey(SDL_SCANCODE_DOWN, Key::Down);
            initTranslateKey(SDL_SCANCODE_LEFT, Key::Left);
            initTranslateKey(SDL_SCANCODE_RIGHT, Key::Right);
            initTranslateKey(SDL_SCANCODE_PAGEUP, Key::PageUp);
            initTranslateKey(SDL_SCANCODE_PAGEDOWN, Key::PageDown);
            initTranslateKey(SDL_SCANCODE_HOME, Key::Home);
            initTranslateKey(SDL_SCANCODE_END, Key::End);
            initTranslateKey(SDL_SCANCODE_PRINTSCREEN, Key::Print);
            initTranslateKey(SDL_SCANCODE_KP_PLUS, Key::Plus);
            initTranslateKey(SDL_SCANCODE_EQUALS, Key::Plus);
            initTranslateKey(SDL_SCANCODE_KP_MINUS, Key::Minus);
            initTranslateKey(SDL_SCANCODE_MINUS, Key::Minus);
            initTranslateKey(SDL_SCANCODE_GRAVE, Key::Tilde);
            initTranslateKey(SDL_SCANCODE_KP_COMMA, Key::Comma);
            initTranslateKey(SDL_SCANCODE_COMMA, Key::Comma);
            initTranslateKey(SDL_SCANCODE_KP_PERIOD, Key::Period);
            initTranslateKey(SDL_SCANCODE_PERIOD, Key::Period);
            initTranslateKey(SDL_SCANCODE_SLASH, Key::Slash);
            initTranslateKey(SDL_SCANCODE_F1, Key::F1);
            initTranslateKey(SDL_SCANCODE_F2, Key::F2);
            initTranslateKey(SDL_SCANCODE_F3, Key::F3);
            initTranslateKey(SDL_SCANCODE_F4, Key::F4);
            initTranslateKey(SDL_SCANCODE_F5, Key::F5);
            initTranslateKey(SDL_SCANCODE_F6, Key::F6);
            initTranslateKey(SDL_SCANCODE_F7, Key::F7);
            initTranslateKey(SDL_SCANCODE_F8, Key::F8);
            initTranslateKey(SDL_SCANCODE_F9, Key::F9);
            initTranslateKey(SDL_SCANCODE_F10, Key::F10);
            initTranslateKey(SDL_SCANCODE_F11, Key::F11);
            initTranslateKey(SDL_SCANCODE_F12, Key::F12);
            initTranslateKey(SDL_SCANCODE_KP_0, Key::NumPad0);
            initTranslateKey(SDL_SCANCODE_KP_1, Key::NumPad1);
            initTranslateKey(SDL_SCANCODE_KP_2, Key::NumPad2);
            initTranslateKey(SDL_SCANCODE_KP_3, Key::NumPad3);
            initTranslateKey(SDL_SCANCODE_KP_4, Key::NumPad4);
            initTranslateKey(SDL_SCANCODE_KP_5, Key::NumPad5);
            initTranslateKey(SDL_SCANCODE_KP_6, Key::NumPad6);
            initTranslateKey(SDL_SCANCODE_KP_7, Key::NumPad7);
            initTranslateKey(SDL_SCANCODE_KP_8, Key::NumPad8);
            initTranslateKey(SDL_SCANCODE_KP_9, Key::NumPad9);
            initTranslateKey(SDL_SCANCODE_0, Key::Key0);
            initTranslateKey(SDL_SCANCODE_1, Key::Key1);
            initTranslateKey(SDL_SCANCODE_2, Key::Key2);
            initTranslateKey(SDL_SCANCODE_3, Key::Key3);
            initTranslateKey(SDL_SCANCODE_4, Key::Key4);
            initTranslateKey(SDL_SCANCODE_5, Key::Key5);
            initTranslateKey(SDL_SCANCODE_6, Key::Key6);
            initTranslateKey(SDL_SCANCODE_7, Key::Key7);
            initTranslateKey(SDL_SCANCODE_8, Key::Key8);
            initTranslateKey(SDL_SCANCODE_9, Key::Key9);
            initTranslateKey(SDL_SCANCODE_A, Key::KeyA);
            initTranslateKey(SDL_SCANCODE_B, Key::KeyB);
            initTranslateKey(SDL_SCANCODE_C, Key::KeyC);
            initTranslateKey(SDL_SCANCODE_D, Key::KeyD);
            initTranslateKey(SDL_SCANCODE_E, Key::KeyE);
            initTranslateKey(SDL_SCANCODE_F, Key::KeyF);
            initTranslateKey(SDL_SCANCODE_G, Key::KeyG);
            initTranslateKey(SDL_SCANCODE_H, Key::KeyH);
            initTranslateKey(SDL_SCANCODE_I, Key::KeyI);
            initTranslateKey(SDL_SCANCODE_J, Key::KeyJ);
            initTranslateKey(SDL_SCANCODE_K, Key::KeyK);
            initTranslateKey(SDL_SCANCODE_L, Key::KeyL);
            initTranslateKey(SDL_SCANCODE_M, Key::KeyM);
            initTranslateKey(SDL_SCANCODE_N, Key::KeyN);
            initTranslateKey(SDL_SCANCODE_O, Key::KeyO);
            initTranslateKey(SDL_SCANCODE_P, Key::KeyP);
            initTranslateKey(SDL_SCANCODE_Q, Key::KeyQ);
            initTranslateKey(SDL_SCANCODE_R, Key::KeyR);
            initTranslateKey(SDL_SCANCODE_S, Key::KeyS);
            initTranslateKey(SDL_SCANCODE_T, Key::KeyT);
            initTranslateKey(SDL_SCANCODE_U, Key::KeyU);
            initTranslateKey(SDL_SCANCODE_V, Key::KeyV);
            initTranslateKey(SDL_SCANCODE_W, Key::KeyW);
            initTranslateKey(SDL_SCANCODE_X, Key::KeyX);
            initTranslateKey(SDL_SCANCODE_Y, Key::KeyY);
            initTranslateKey(SDL_SCANCODE_Z, Key::KeyZ);

            memset(s_translateGamepad, uint8_t(Key::Count), sizeof(s_translateGamepad));
            initTranslateGamepad(SDL_CONTROLLER_BUTTON_A, Key::GamepadA);
            initTranslateGamepad(SDL_CONTROLLER_BUTTON_B, Key::GamepadB);
            initTranslateGamepad(SDL_CONTROLLER_BUTTON_X, Key::GamepadX);
            initTranslateGamepad(SDL_CONTROLLER_BUTTON_Y, Key::GamepadY);
            initTranslateGamepad(SDL_CONTROLLER_BUTTON_LEFTSTICK, Key::GamepadThumbL);
            initTranslateGamepad(SDL_CONTROLLER_BUTTON_RIGHTSTICK, Key::GamepadThumbR);
            initTranslateGamepad(SDL_CONTROLLER_BUTTON_LEFTSHOULDER, Key::GamepadShoulderL);
            initTranslateGamepad(SDL_CONTROLLER_BUTTON_RIGHTSHOULDER, Key::GamepadShoulderR);
            initTranslateGamepad(SDL_CONTROLLER_BUTTON_DPAD_UP, Key::GamepadUp);
            initTranslateGamepad(SDL_CONTROLLER_BUTTON_DPAD_DOWN, Key::GamepadDown);
            initTranslateGamepad(SDL_CONTROLLER_BUTTON_DPAD_LEFT, Key::GamepadLeft);
            initTranslateGamepad(SDL_CONTROLLER_BUTTON_DPAD_RIGHT, Key::GamepadRight);
            initTranslateGamepad(SDL_CONTROLLER_BUTTON_BACK, Key::GamepadBack);
            initTranslateGamepad(SDL_CONTROLLER_BUTTON_START, Key::GamepadStart);
            initTranslateGamepad(SDL_CONTROLLER_BUTTON_GUIDE, Key::GamepadGuide);

            memset(s_translateGamepadAxis, uint8_t(GamepadAxis::Count), sizeof(s_translateGamepadAxis));
            initTranslateGamepadAxis(SDL_CONTROLLER_AXIS_LEFTX, GamepadAxis::LeftX);
            initTranslateGamepadAxis(SDL_CONTROLLER_AXIS_LEFTY, GamepadAxis::LeftY);
            initTranslateGamepadAxis(SDL_CONTROLLER_AXIS_TRIGGERLEFT, GamepadAxis::LeftZ);
            initTranslateGamepadAxis(SDL_CONTROLLER_AXIS_RIGHTX, GamepadAxis::RightX);
            initTranslateGamepadAxis(SDL_CONTROLLER_AXIS_RIGHTY, GamepadAxis::RightY);
            initTranslateGamepadAxis(SDL_CONTROLLER_AXIS_TRIGGERRIGHT, GamepadAxis::RightZ);
        }

        int run()
        {

            SDL_Init(SDL_INIT_GAMECONTROLLER);

            m_window = SDL_CreateWindow("Arena"
                , SDL_WINDOWPOS_UNDEFINED
                , SDL_WINDOWPOS_UNDEFINED
                , m_width
                , m_height
                , SDL_WINDOW_SHOWN
                | SDL_WINDOW_RESIZABLE
            );

            bgfx::sdlSetWindow(m_window);
            setWindowSize(m_width, m_height, true);

            bgfx::renderFrame();

            m_thread.init(thread_proc);

            bool exit = false;

            SDL_Event event;
            while (!exit)
            {
                bgfx::renderFrame();

                while (SDL_PollEvent(&event))
                    switch (event.type)
                    {
                    case SDL_QUIT:
                    {    
                        m_eventQueue.postExitEvent();
                        exit = true;
                        break;
                    }
                    case SDL_MOUSEMOTION:
                    {
                        const SDL_MouseMotionEvent& mev = event.motion;
						 mev.xrel;
                        m_mx = mev.x;
                        m_my = mev.y;
						m_mrx = mev.xrel;
						m_mry = mev.yrel;
                        m_eventQueue.postMouseEvent(m_window, m_mx, m_my, m_mz, m_mrx, m_mry);
                        break;
                    }
                    

                    case SDL_MOUSEBUTTONDOWN:
                    case SDL_MOUSEBUTTONUP:
                    {
                        const SDL_MouseButtonEvent& mev = event.button;

                        MouseButton::Enum button;
                        switch (mev.button)
                        {
                        default:
                        case SDL_BUTTON_LEFT:   button = MouseButton::Left;   break;
                        case SDL_BUTTON_MIDDLE: button = MouseButton::Middle; break;
                        case SDL_BUTTON_RIGHT:  button = MouseButton::Right;  break;
                        }

                        m_eventQueue.postMouseEvent(m_window
                            , mev.x
                            , mev.y
                            , 0
                            , button
                            , mev.type == SDL_MOUSEBUTTONDOWN
                        );
                        
                    }
                    break;

                    case SDL_MOUSEWHEEL:
                    {
                        const SDL_MouseWheelEvent& mev = event.wheel;
                        m_mz += mev.y;

                        m_eventQueue.postMouseEvent(m_window, m_mx, m_my, m_mz, m_mrx, m_mry);

                    }
                    break;

                    case SDL_TEXTINPUT:
                    {
                        const SDL_TextInputEvent& tev = event.text;
                        m_eventQueue.postCharEvent(m_window, 1, (const uint8_t*)tev.text);
                    }
                    break;

                    case SDL_KEYDOWN:
                    {
                        const SDL_KeyboardEvent& kev = event.key;

                        uint8_t modifiers = translateKeyModifiers(kev.keysym.mod);
                        Key::Enum key = translateKey(kev.keysym.scancode);

                        /// If you only press (e.g.) 'shift' and nothing else, then key == 'shift', modifier == 0.
                        /// Further along, pressing 'shift' + 'ctrl' would be: key == 'shift', modifier == 'ctrl.
                        if (0 == key && 0 == modifiers)
                        {
                            modifiers = translateKeyModifierPress(kev.keysym.scancode);
                        }

                        /// TODO: These keys are not captured by SDL_TEXTINPUT. Should be probably handled by SDL_TEXTEDITING. This is a workaround for now.
                        if (Key::Esc == key)
                        {
                            uint8_t pressedChar[4];
                            pressedChar[0] = 0x1b;
                            m_eventQueue.postCharEvent(m_window, 1, pressedChar);
                        }
                        else if (Key::Return == key)
                        {
                            uint8_t pressedChar[4];
                            pressedChar[0] = 0x0d;
                            m_eventQueue.postCharEvent(m_window, 1, pressedChar);
                        }
                        else if (Key::Backspace == key)
                        {
                            uint8_t pressedChar[4];
                            pressedChar[0] = 0x08;
                            m_eventQueue.postCharEvent(m_window, 1, pressedChar);
                        }
                    }
                    case SDL_KEYUP:
                    {
                        const SDL_KeyboardEvent& kev = event.key;

                        uint8_t modifiers = translateKeyModifiers(kev.keysym.mod);
                        Key::Enum key = translateKey(kev.keysym.scancode);
                        m_eventQueue.postKeyEvent(m_window, key, modifiers, kev.state == SDL_PRESSED);
                        
                    }
                    break;

                    case SDL_WINDOWEVENT:
                    {
                        const SDL_WindowEvent& wev = event.window;
                        switch (wev.event)
                        {
                        case SDL_WINDOWEVENT_RESIZED:
                        case SDL_WINDOWEVENT_SIZE_CHANGED:
                        {
                            setWindowSize(wev.data1, wev.data2);
                        }
                        break;

                        case SDL_WINDOWEVENT_SHOWN:
                        case SDL_WINDOWEVENT_HIDDEN:
                        case SDL_WINDOWEVENT_EXPOSED:
                        case SDL_WINDOWEVENT_MOVED:
                        case SDL_WINDOWEVENT_MINIMIZED:
                        case SDL_WINDOWEVENT_MAXIMIZED:
                        case SDL_WINDOWEVENT_RESTORED:
                        case SDL_WINDOWEVENT_ENTER:
                        case SDL_WINDOWEVENT_LEAVE:
                        case SDL_WINDOWEVENT_FOCUS_GAINED:
                        case SDL_WINDOWEVENT_FOCUS_LOST:
                            break;

                        case SDL_WINDOWEVENT_CLOSE:
                        {
                            m_eventQueue.postExitEvent();
                            exit = true;
                        }
                        break;
                        }
                    }
                    break;

                    }

            }

            while (bgfx::RenderFrame::NoContext != bgfx::renderFrame()) {};

            m_thread.shutdown();

            SDL_DestroyWindow(m_window);
            SDL_Quit();

            return m_thread.getExitCode();
        }

        bx::Thread m_thread;

        int32_t m_width;
        int32_t m_height;

        int32_t m_mx;
        int32_t m_my;
        int32_t m_mz;
		int32_t m_mrx;
		int32_t m_mry;
        SDL_Window* m_window;

        EventQueue m_eventQueue;
    };

    static Context s_ctx;
    
    void setWindowSize(uint32_t width, uint32_t height, bool force)
    {
        if (width != s_ctx.m_width
            || height != s_ctx.m_height
            || force)
        {
            s_ctx.m_width = width;
            s_ctx.m_height = height;

            SDL_SetWindowSize(s_ctx.m_window, width, height);
			SDL_SetWindowGrab(s_ctx.m_window, SDL_bool::SDL_TRUE);
			SDL_SetWindowTitle(s_ctx.m_window, "Areenajabat");
			SDL_SetRelativeMouseMode(SDL_bool::SDL_TRUE);
			SDL_ShowCursor(0);
            s_ctx.m_eventQueue.postSizeEvent(s_ctx.m_window, width, height);
        }
    }

    const Event* poll()
    {
        return s_ctx.m_eventQueue.poll();
    }

    void release(const Event* _event)
    {
        s_ctx.m_eventQueue.release(_event);
    }

    int32_t thread_proc(void*)
    {
        bgfx::init();

        inputInit();

		App& app = App::instance();

		app.init(s_ctx.m_width, s_ctx.m_height);

        while (!app.update());

        SDL_Event event;
        SDL_QuitEvent& qev = event.quit;
        qev.type = SDL_QUIT;
        SDL_PushEvent(&event);

        app.shutdown();

        bgfx::shutdown();

        inputShutdown();

        return 0;
    }
}

int main(int argc, char** argv)
{
    using namespace arena;
    (void)argc;
    (void)argv;
    s_ctx.run();

    return 0;
}
