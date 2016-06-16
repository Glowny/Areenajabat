#include "app.h"
#include <bgfx/bgfx.h>
#include "input/input.h"
#include "input/event.h"

namespace arena
{
    static bool s_exit = false;
    static uint32_t s_reset = BGFX_RESET_NONE;

    static void cmdExit(const void*)
    {
        s_exit = true;
    }

    static const InputBinding s_bindings[] =
    {
        { arena::Key::KeyQ, arena::Modifier::LeftCtrl, 0, cmdExit, "exit" },
        INPUT_BINDING_END
    };

    void App::init(int32_t width, int32_t height)
    {
        this->width = width;
        this->height = height;

        inputInit();
        inputAddBindings("bindings", s_bindings);

        bgfx::reset(width, height, s_reset);
        //bgfx::setDebug(s_reset);

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
            bgfx::reset(width, height, reset);
            inputSetMouseResolution(uint16_t(width), uint16_t(height));
        }

        // Set view 0 default viewport.
        bgfx::setViewRect(0, 0, 0, uint16_t(width), uint16_t(height));

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