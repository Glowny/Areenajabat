#pragma once

#include <stdint.h>
#include <glm/glm.hpp>
#include <bx/bx.h>

namespace bgfx { struct ProgramHandle; };

namespace arena
{
    class TextureResource;

    struct Sprite
    {
        // DirectX origin is different
        static const bool OriginBottomLeft = BX_PLATFORM_WINDOWS != 0 ? true : false;

        void setColor(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = 255);
        void setColor(uint32_t rgba);

        Sprite(TextureResource* texture)
            : m_position(0, 0),
              m_origin(0, 0),
              m_scale(1, 1),
              m_abgr(0xFFFFFFFF), // white
              m_angle(0.f),
              m_texture(texture)
        {

        }

        void submit(uint8_t view, bgfx::ProgramHandle program, uint64_t state) const;

        glm::vec2 m_position;
        glm::vec2 m_origin;
        glm::vec2 m_scale;
        uint32_t m_abgr;
        float m_angle;
        TextureResource* m_texture;
    };
}