#include "input.h"
#include <bx/bx.h>
#include <bgfx/bgfx.h>
#include <bx/ringbuffer.h>
#include <string>
#include <unordered_map>

namespace arena
{
    static const char* s_keyName[] =
    {
        "None",
        "Esc",
        "Return",
        "Tab",
        "Space",
        "Backspace",
        "Up",
        "Down",
        "Left",
        "Right",
        "Insert",
        "Delete",
        "Home",
        "End",
        "PageUp",
        "PageDown",
        "Print",
        "Plus",
        "Minus",
        "LeftBracket",
        "RightBracket",
        "Semicolon",
        "Quote",
        "Comma",
        "Period",
        "Slash",
        "Backslash",
        "Tilde",
        "F1",
        "F2",
        "F3",
        "F4",
        "F5",
        "F6",
        "F7",
        "F8",
        "F9",
        "F10",
        "F11",
        "F12",
        "NumPad0",
        "NumPad1",
        "NumPad2",
        "NumPad3",
        "NumPad4",
        "NumPad5",
        "NumPad6",
        "NumPad7",
        "NumPad8",
        "NumPad9",
        "Key0",
        "Key1",
        "Key2",
        "Key3",
        "Key4",
        "Key5",
        "Key6",
        "Key7",
        "Key8",
        "Key9",
        "KeyA",
        "KeyB",
        "KeyC",
        "KeyD",
        "KeyE",
        "KeyF",
        "KeyG",
        "KeyH",
        "KeyI",
        "KeyJ",
        "KeyK",
        "KeyL",
        "KeyM",
        "KeyN",
        "KeyO",
        "KeyP",
        "KeyQ",
        "KeyR",
        "KeyS",
        "KeyT",
        "KeyU",
        "KeyV",
        "KeyW",
        "KeyX",
        "KeyY",
        "KeyZ",
        "GamepadA",
        "GamepadB",
        "GamepadX",
        "GamepadY",
        "GamepadThumbL",
        "GamepadThumbR",
        "GamepadShoulderL",
        "GamepadShoulderR",
        "GamepadUp",
        "GamepadDown",
        "GamepadLeft",
        "GamepadRight",
        "GamepadBack",
        "GamepadStart",
        "GamepadGuide",
    };
    BX_STATIC_ASSERT(Key::Count == BX_COUNTOF(s_keyName));

    const char* getName(Key::Enum _key)
    {
        BX_CHECK(_key < Key::Count, "Invalid key %d.", _key);
        return s_keyName[_key];
    }

    struct Mouse
    {
        Mouse()
            : m_width(1280)
            , m_height(720)
            , m_wheelDelta(120)
            , m_lock(false)
        {
        }

        void reset()
        {
            if (m_lock)
            {
                m_norm[0] = 0.0f;
                m_norm[1] = 0.0f;
                m_norm[2] = 0.0f;
            }

            memset(m_buttons, 0, sizeof(m_buttons));
        }

        void setResolution(uint16_t _width, uint16_t _height)
        {
            m_width = _width;
            m_height = _height;
        }

        void setPos(int32_t _mx, int32_t _my, int32_t _mz)
        {
            m_absolute[0] = _mx;
            m_absolute[1] = _my;
            m_absolute[2] = _mz;
            m_norm[0] = float(_mx) / float(m_width);
            m_norm[1] = float(_my) / float(m_height);
            m_norm[2] = float(_mz) / float(m_wheelDelta);
        }

        void setButtonState(arena::MouseButton::Enum _button, uint8_t _state)
        {
            m_buttons[_button] = _state;
        }

        int32_t m_absolute[3];
        float m_norm[3];
        int32_t m_wheel;
        uint8_t m_buttons[arena::MouseButton::Count];
        uint16_t m_width;
        uint16_t m_height;
        uint16_t m_wheelDelta;
        bool m_lock;
    };

    struct Keyboard
    {
        Keyboard()
            : m_ring(BX_COUNTOF(m_char))
        {
        }

        void reset()
        {
            memset(m_key, 0, sizeof(m_key));
            memset(m_once, 0xff, sizeof(m_once));
        }

        static uint32_t encodeKeyState(uint8_t _modifiers, bool _down)
        {
            uint32_t state = 0;
            state |= uint32_t(_modifiers) << 16;
            state |= uint32_t(_down) << 8;
            return state;
        }

        static bool decodeKeyState(uint32_t _state, uint8_t& _modifiers)
        {
            _modifiers = (_state >> 16) & 0xff;
            return 0 != ((_state >> 8) & 0xff);
        }

        void setKeyState(arena::Key::Enum _key, uint8_t _modifiers, bool _down)
        {
            m_key[_key] = encodeKeyState(_modifiers, _down);
            m_once[_key] = false;
        }

        bool getKeyState(arena::Key::Enum _key, uint8_t* _modifiers)
        {
            uint8_t modifiers;
            _modifiers = NULL == _modifiers ? &modifiers : _modifiers;

            return decodeKeyState(m_key[_key], *_modifiers);
        }

        uint8_t getModifiersState()
        {
            uint8_t modifiers = 0;
            for (uint32_t ii = 0; ii < arena::Key::Count; ++ii)
            {
                modifiers |= (m_key[ii] >> 16) & 0xff;
            }
            return modifiers;
        }

        void pushChar(uint8_t _len, const uint8_t _char[4])
        {
            for (uint32_t len = m_ring.reserve(4)
                ; len < _len
                ; len = m_ring.reserve(4)
                )
            {
                popChar();
            }

            memcpy(&m_char[m_ring.m_current], _char, 4);
            m_ring.commit(4);
        }

        const uint8_t* popChar()
        {
            if (0 < m_ring.available())
            {
                uint8_t* utf8 = &m_char[m_ring.m_read];
                m_ring.consume(4);
                return utf8;
            }

            return NULL;
        }

        void charFlush()
        {
            m_ring.m_current = 0;
            m_ring.m_write = 0;
            m_ring.m_read = 0;
        }

        uint32_t m_key[256];
        bool m_once[256];

        bx::RingBufferControl m_ring;
        uint8_t m_char[256];
    };

    struct Input
    {
        Input()
        {
            reset();
        }

        ~Input()
        {
        }

        void addBindings(const char* _name, const InputBinding* _bindings)
        {
            m_inputBindingsMap.insert(std::make_pair(std::string(_name), _bindings));
        }

        void removeBindings(const char* _name)
        {
            InputBindingMap::iterator it = m_inputBindingsMap.find(std::string(_name));
            if (it != m_inputBindingsMap.end())
            {
                m_inputBindingsMap.erase(it);
            }
        }

        void process(const InputBinding* _bindings)
        {
            for (const InputBinding* binding = _bindings; binding->m_key != arena::Key::None; ++binding)
            {
                uint8_t modifiers;
                bool down = Keyboard::decodeKeyState(m_keyboard.m_key[binding->m_key], modifiers);

                if (binding->m_flags == 1)
                {
                    if (down)
                    {
                        if (modifiers == binding->m_modifiers
                            && !m_keyboard.m_once[binding->m_key])
                        {
                            binding->m_fn(binding->m_userData);

                            m_keyboard.m_once[binding->m_key] = true;
                        }
                    }
                    else
                    {
                        m_keyboard.m_once[binding->m_key] = false;
                    }
                }
                else
                {
                    if (down
                        &&  modifiers == binding->m_modifiers)
                    {
                        binding->m_fn(binding->m_userData);
                    }
                }
            }
        }

        void process()
        {
            for (InputBindingMap::const_iterator it = m_inputBindingsMap.begin(); it != m_inputBindingsMap.end(); ++it)
            {
                process(it->second);
            }
        }

        void reset()
        {
            m_mouse.reset();
            m_keyboard.reset();
            /*for (uint32_t ii = 0; ii < BX_COUNTOF(m_gamepad); ++ii)
            {
                m_gamepad[ii].reset();
            }*/
        }

        typedef std::unordered_map<std::string, const InputBinding*> InputBindingMap;
        InputBindingMap m_inputBindingsMap;
        Mouse m_mouse;
        Keyboard m_keyboard;
        //Gamepad m_gamepad[ENTRY_CONFIG_MAX_GAMEPADS];
    };

    static Input* s_input;

    void inputInit()
    {
        s_input = new Input;
    }

    void inputShutdown()
    {
        delete s_input;
    }

    void inputAddBindings(const char* _name, const InputBinding* _bindings)
    {
        s_input->addBindings(_name, _bindings);
    }

    void inputRemoveBindings(const char* _name)
    {
        s_input->removeBindings(_name);
    }

    void inputProcess()
    {
        s_input->process();
    }

    void inputSetMouseResolution(uint16_t _width, uint16_t _height)
    {
        s_input->m_mouse.setResolution(_width, _height);
    }

    void inputSetKeyState(arena::Key::Enum _key, uint8_t _modifiers, bool _down)
    {
        s_input->m_keyboard.setKeyState(_key, _modifiers, _down);
    }

    bool inputGetKeyState(arena::Key::Enum _key, uint8_t* _modifiers)
    {
        return s_input->m_keyboard.getKeyState(_key, _modifiers);
    }

    uint8_t inputGetModifiersState()
    {
        return s_input->m_keyboard.getModifiersState();
    }

    void inputChar(uint8_t _len, const uint8_t _char[4])
    {
        s_input->m_keyboard.pushChar(_len, _char);
    }

    const uint8_t* inputGetChar()
    {
        return s_input->m_keyboard.popChar();
    }

    void inputCharFlush()
    {
        s_input->m_keyboard.charFlush();
    }

    void inputSetMousePos(int32_t _mx, int32_t _my, int32_t _mz)
    {
        s_input->m_mouse.setPos(_mx, _my, _mz);
    }

    void inputSetMouseButtonState(arena::MouseButton::Enum _button, uint8_t _state)
    {
        s_input->m_mouse.setButtonState(_button, _state);
    }

    void inputGetMouse(float _mouse[3])
    {
        _mouse[0] = s_input->m_mouse.m_norm[0];
        _mouse[1] = s_input->m_mouse.m_norm[1];
        _mouse[2] = s_input->m_mouse.m_norm[2];
        s_input->m_mouse.m_norm[0] = 0.0f;
        s_input->m_mouse.m_norm[1] = 0.0f;
        s_input->m_mouse.m_norm[2] = 0.0f;
    }

    void inputGetMouseAbsolute(int32_t mouse[3])
    {
        mouse[0] = s_input->m_mouse.m_absolute[0];
        mouse[1] = s_input->m_mouse.m_absolute[1];
        mouse[2] = s_input->m_mouse.m_absolute[2];
    }

    bool inputIsMouseLocked()
    {
        return s_input->m_mouse.m_lock;
    }

    /*
    void inputSetGamepadAxis(entry::GamepadHandle _handle, entry::GamepadAxis::Enum _axis, int32_t _value)
    {
        s_input->m_gamepad[_handle.idx].setAxis(_axis, _value);
    }

    int32_t inputGetGamepadAxis(entry::GamepadHandle _handle, entry::GamepadAxis::Enum _axis)
    {
        return s_input->m_gamepad[_handle.idx].getAxis(_axis);
    }*/
}