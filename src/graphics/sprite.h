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
        Sprite(TextureResource* texture)
            : m_position(0, 0),
              m_origin(0, 0),
              m_scale(1, 1),
              m_abgr(0xFFFFFFFF), // white
              m_angle(0.f),
              m_texture(texture)
        {

        }

        glm::vec2 m_position;
        glm::vec2 m_origin;
        glm::vec2 m_scale;
        uint32_t m_abgr;
        float m_angle;
        TextureResource* m_texture;
    };
}