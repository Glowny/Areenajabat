#include "app.h"
#include <bgfx/bgfx.h>
#include "input/input.h"
#include "input/event.h"

namespace arena
{
    static bool s_exit = false;

    static void cmdExit(const void*)
    {
        s_exit = true;
    }

    static const InputBinding s_bindings[] =
    {
        { arena::Key::KeyQ, arena::Modifier::LeftCtrl, 1, cmdExit, "exit" },
        INPUT_BINDING_END
    };

    void App::init()
    {
        inputInit();
        inputAddBindings("bindings", s_bindings);

        bgfx::reset(1280, 720, BGFX_DEBUG_TEXT);

        // Enable debug text.
        bgfx::setDebug(BGFX_DEBUG_TEXT);

        // Set view 0 clear state.
        bgfx::setViewClear(0
            , BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH
            , 0x303030ff
            , 1.0f
            , 0
            );
    }

    bool App::update()
    {
        const Event* ev;

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
                        }
                        else
                        {
                            inputSetMouseButtonState(mouse->m_button, mouse->m_down);
                        }

                        /*if (NULL != _mouse
                            && !mouseLock)
                        {
                            if (mouse->m_move)
                            {
                                _mouse->m_mx = mouse->m_mx;
                                _mouse->m_my = mouse->m_my;
                                _mouse->m_mz = mouse->m_mz;
                            }
                            else
                            {
                                _mouse->m_buttons[mouse->m_button] = mouse->m_down;
                            }
                        }*/
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
                        /*_width = size->m_width;
                        _height = size->m_height;
                        _reset = !s_reset; // force reset*/ // TODO
                    }
                    break;
                    default:
                        break;
                }
            }

            inputProcess();

        } while (ev != NULL);

        // Set view 0 default viewport.
        bgfx::setViewRect(0, 0, 0, uint16_t(1280), uint16_t(720));
        bgfx::touch(0);
        bgfx::frame();

        return s_exit;
    }

    void App::shutdown()
    {
        inputRemoveBindings("bindings");
        inputShutdown();
    }
}