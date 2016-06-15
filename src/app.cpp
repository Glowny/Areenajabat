#include "app.h"
#include <bgfx/bgfx.h>

namespace arena
{
    void App::init()
    {
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

    void App::update()
    {
        // Set view 0 default viewport.
        bgfx::setViewRect(0, 0, 0, uint16_t(1280), uint16_t(720));
        bgfx::touch(0);
        bgfx::frame();
    }

    void App::shutdown()
    {

    }
}