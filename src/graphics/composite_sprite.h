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
        float m_rotation;
        float m_depth;

        std::vector<CompositeSprite*> m_children;
    };
}