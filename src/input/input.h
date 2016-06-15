#pragma once

#include <stdint.h>
#include <string.h> // memset

namespace arena
{
    struct WindowHandle { uint16_t idx; };
    inline bool isValid(WindowHandle _handle) { return UINT16_MAX != _handle.idx; }

    struct GamepadHandle { uint16_t idx; };
    inline bool isValid(GamepadHandle _handle) { return UINT16_MAX != _handle.idx; }
}

namespace arena
{

    struct MouseButton
    {
        enum Enum
        {
            None,
            Left,
            Middle,
            Right,

            Count
        };
    };

    struct GamepadAxis
    {
        enum Enum
        {
            LeftX,
            LeftY,
            LeftZ,
            RightX,
            RightY,
            RightZ,

            Count
        };
    };

    struct Modifier
    {
        enum Enum
        {
            None = 0,
            LeftAlt = 0x01,
            RightAlt = 0x02,
            LeftCtrl = 0x04,
            RightCtrl = 0x08,
            LeftShift = 0x10,
            RightShift = 0x20,
            LeftMeta = 0x40,
            RightMeta = 0x80,
        };
    };

    struct Key
    {
        enum Enum
        {
            None = 0,
            Esc,
            Return,
            Tab,
            Space,
            Backspace,
            Up,
            Down,
            Left,
            Right,
            Insert,
            Delete,
            Home,
            End,
            PageUp,
            PageDown,
            Print,
            Plus,
            Minus,
            LeftBracket,
            RightBracket,
            Semicolon,
            Quote,
            Comma,
            Period,
            Slash,
            Backslash,
            Tilde,
            F1,
            F2,
            F3,
            F4,
            F5,
            F6,
            F7,
            F8,
            F9,
            F10,
            F11,
            F12,
            NumPad0,
            NumPad1,
            NumPad2,
            NumPad3,
            NumPad4,
            NumPad5,
            NumPad6,
            NumPad7,
            NumPad8,
            NumPad9,
            Key0,
            Key1,
            Key2,
            Key3,
            Key4,
            Key5,
            Key6,
            Key7,
            Key8,
            Key9,
            KeyA,
            KeyB,
            KeyC,
            KeyD,
            KeyE,
            KeyF,
            KeyG,
            KeyH,
            KeyI,
            KeyJ,
            KeyK,
            KeyL,
            KeyM,
            KeyN,
            KeyO,
            KeyP,
            KeyQ,
            KeyR,
            KeyS,
            KeyT,
            KeyU,
            KeyV,
            KeyW,
            KeyX,
            KeyY,
            KeyZ,

            GamepadA,
            GamepadB,
            GamepadX,
            GamepadY,
            GamepadThumbL,
            GamepadThumbR,
            GamepadShoulderL,
            GamepadShoulderR,
            GamepadUp,
            GamepadDown,
            GamepadLeft,
            GamepadRight,
            GamepadBack,
            GamepadStart,
            GamepadGuide,

            Count
        };
    };

    struct Suspend
    {
        enum Enum
        {
            WillSuspend,
            DidSuspend,
            WillResume,
            DidResume,

            Count
        };
    };

    struct MouseState
    {
        MouseState()
            : m_mx(0)
            , m_my(0)
            , m_mz(0)
        {
            for (uint32_t ii = 0; ii < MouseButton::Count; ++ii)
            {
                m_buttons[ii] = MouseButton::None;
            }
        }

        int32_t m_mx;
        int32_t m_my;
        int32_t m_mz;
        uint8_t m_buttons[MouseButton::Count];
    };

    struct GamepadState
    {
        GamepadState()
        {
            memset(m_axis, 0, sizeof(m_axis));
        }

        int32_t m_axis[GamepadAxis::Count];
    };

    struct WindowState
    {
        WindowState()
            : m_width(0)
            , m_height(0)
            , m_nwh(NULL)
        {
            m_handle.idx = UINT16_MAX;
        }

        WindowHandle m_handle;
        uint32_t     m_width;
        uint32_t     m_height;
        MouseState   m_mouse;
        void*        m_nwh;
    };

    const char* getName(Key::Enum _key);

    WindowHandle createWindow(int32_t _x, int32_t _y, uint32_t _width, uint32_t _height, const char* _title = "");
    void destroyWindow(WindowHandle _handle);
    void setWindowPos(WindowHandle _handle, int32_t _x, int32_t _y);
    void setWindowSize(WindowHandle _handle, uint32_t _width, uint32_t _height);
    void setWindowTitle(WindowHandle _handle, const char* _title);
    void toggleWindowFrame(WindowHandle _handle);
    void toggleFullscreen(WindowHandle _handle);
    void setMouseLock(WindowHandle _handle, bool _lock);

}