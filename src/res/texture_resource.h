#pragma once

#include <stdint.h>
#include <bgfx/bgfx.h>

namespace arena
{
    class TextureResource
    {
    public:
        bgfx::TextureHandle handle;
        bgfx::TextureFormat format;
        uint16_t width;
        uint16_t height;
    };
}