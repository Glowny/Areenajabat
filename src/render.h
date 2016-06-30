#pragma once
#include <glm/glm.hpp>
#include "graphics/sprite_effects.h"

namespace arena
{
    class TextureResource;

    void draw(const TextureResource* texture, glm::vec4* src, uint32_t color, const glm::vec2& position, 
        const glm::vec2& origin, const glm::vec2& scale, uint8_t effect, float angle, float depth);
}

