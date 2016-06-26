#pragma once
#include <glm/glm.hpp>

namespace arena
{
    class TextureResource;

    void draw(const TextureResource* texture, glm::vec4* src, uint32_t color, const glm::vec2& position, const glm::vec2& origin, const glm::vec2& scale, float angle, float depth);
}

