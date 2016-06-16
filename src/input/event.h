#pragma once

#include "input.h"
#include <bx/spscqueue.h>
#include <SDL2/SDL.h>


#define ENTRY_IMPLEMENT_EVENT(_class, _type) \
_class(SDL_Window* _handle) : Event(_type, _handle) {}

namespace arena
{
    struct Event
    {
        enum Enum
        {
            Axis,
            Char,
            Exit,
            Gamepad,
            Key,
            Mouse,
            Size,
            Window,
            Suspend,
        };

        Event(Enum _type)
            : m_type(_type)
        {
            m_window = nullptr;
        }

        Event(Enum _type, SDL_Window* _handle)
            : m_type(_type)
            , m_window(_handle)
        {
        }

        Event::Enum m_type;
        SDL_Window* m_window;
    };

    struct KeyEvent : public Event
    {
        ENTRY_IMPLEMENT_EVENT(KeyEvent, Event::Key);

        Key::Enum m_key;
        uint8_t m_modifiers;
        bool m_down;
    };

    struct CharEvent : public Event
    {
        ENTRY_IMPLEMENT_EVENT(CharEvent, Event::Char);

        uint8_t m_len;
        uint8_t m_char[4];
    };

    struct MouseEvent : public Event
    {
        ENTRY_IMPLEMENT_EVENT(MouseEvent, Event::Mouse);

        int32_t m_mx;
        int32_t m_my;
        int32_t m_mz;
        MouseButton::Enum m_button;
        bool m_down;
        bool m_move;
    };

    struct SizeEvent : public Event
    {
        ENTRY_IMPLEMENT_EVENT(SizeEvent, Event::Size);

        uint32_t m_width;
        uint32_t m_height;
    };

    struct WindowEvent : public Event
    {
        ENTRY_IMPLEMENT_EVENT(WindowEvent, Event::Window);

        void* m_nwh;
    };

    struct SuspendEvent : public Event
    {
        ENTRY_IMPLEMENT_EVENT(SuspendEvent, Event::Suspend);

        Suspend::Enum m_state;
    };

    class EventQueue
    {
    public:
        ~EventQueue()
        {
            for (const Event* ev = poll(); NULL != ev; ev = poll())
            {
                release(ev);
            }
        }

        /*
        void postAxisEvent(SDL_Window* _handle, GamepadHandle _gamepad, GamepadAxis::Enum _axis, int32_t _value)
        {
            AxisEvent* ev = new AxisEvent(_handle);
            ev->m_gamepad = _gamepad;
            ev->m_axis = _axis;
            ev->m_value = _value;
            m_queue.push(ev);
        }
        */

        void postCharEvent(SDL_Window* _handle, uint8_t _len, const uint8_t _char[4])
        {
            CharEvent* ev = new CharEvent(_handle);
            ev->m_len = _len;
            memcpy(ev->m_char, _char, 4);
            m_queue.push(ev);
        }

        void postExitEvent()
        {
            Event* ev = new Event(Event::Exit);
            m_queue.push(ev);
        }

        /*
        void postGamepadEvent(WindowHandle _handle, GamepadHandle _gamepad, bool _connected)
        {
            GamepadEvent* ev = new GamepadEvent(_handle);
            ev->m_gamepad = _gamepad;
            ev->m_connected = _connected;
            m_queue.push(ev);
        }*/

        void postKeyEvent(SDL_Window* _handle, Key::Enum _key, uint8_t _modifiers, bool _down)
        {
            KeyEvent* ev = new KeyEvent(_handle);
            ev->m_key = _key;
            ev->m_modifiers = _modifiers;
            ev->m_down = _down;
            m_queue.push(ev);
        }

        void postMouseEvent(SDL_Window* _handle, int32_t _mx, int32_t _my, int32_t _mz)
        {
            MouseEvent* ev = new MouseEvent(_handle);
            ev->m_mx = _mx;
            ev->m_my = _my;
            ev->m_mz = _mz;
            ev->m_button = MouseButton::None;
            ev->m_down = false;
            ev->m_move = true;
            m_queue.push(ev);
        }

        void postMouseEvent(SDL_Window* _handle, int32_t _mx, int32_t _my, int32_t _mz, MouseButton::Enum _button, bool _down)
        {
            MouseEvent* ev = new MouseEvent(_handle);
            ev->m_mx = _mx;
            ev->m_my = _my;
            ev->m_mz = _mz;
            ev->m_button = _button;
            ev->m_down = _down;
            ev->m_move = false;
            m_queue.push(ev);
        }

        void postSizeEvent(SDL_Window* _handle, uint32_t _width, uint32_t _height)
        {
            SizeEvent* ev = new SizeEvent(_handle);
            ev->m_width = _width;
            ev->m_height = _height;
            m_queue.push(ev);
        }

        void postWindowEvent(SDL_Window* _handle, void* _nwh = NULL)
        {
            WindowEvent* ev = new WindowEvent(_handle);
            ev->m_nwh = _nwh;
            m_queue.push(ev);
        }

        void postSuspendEvent(SDL_Window* _handle, Suspend::Enum _state)
        {
            SuspendEvent* ev = new SuspendEvent(_handle);
            ev->m_state = _state;
            m_queue.push(ev);
        }

        const Event* poll()
        {
            Event* ev = m_queue.peek();
            if (NULL == ev)
            {
                return NULL;
            }
            
            return m_queue.pop();
        }

        void release(const Event* _event) const
        {
            delete _event;
        }

    private:
        bx::SpScUnboundedQueue<Event> m_queue;
    };
}