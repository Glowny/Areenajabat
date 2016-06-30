#pragma once

#include <glm/glm.hpp>
#include <vector>

namespace arena
{
    class TextureResource;

    class CompositeSprite
    {
    public:
        CompositeSprite(TextureResource* texture);
        void render();
    private:
        void render(const glm::mat4& parentmtx);
    public:
        TextureResource* m_texture;
        glm::vec2 m_position;
        glm::vec2 m_origin;
        glm::vec2 m_scale;
        float m_rotation;
        float m_depth;

        // after transfroms, HAX
        glm::vec2 m_globalPosition;

        std::vector<CompositeSprite*> m_children;
    };
}