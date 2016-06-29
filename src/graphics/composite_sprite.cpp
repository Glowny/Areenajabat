#include "composite_sprite.h"
#include "../res/texture_resource.h"
#include "../utils/math.h"
#include <glm/gtc/matrix_transform.hpp>
#include "../render.h"

namespace arena
{
    CompositeSprite::CompositeSprite(TextureResource* texture)
        : m_texture(texture),
        m_position(0, 0),
        m_origin(0, 0),
        m_rotation(0.f),
        m_depth(0.f)
    {

    }

    void CompositeSprite::render()
    {
        render(glm::mat4(1.f));
    }

    void CompositeSprite::render(const glm::mat4& parentmtx)
    {
        glm::mat4 globalmtx = parentmtx
            * glm::translate(glm::mat4(1.f), glm::vec3(m_position.x, m_position.y, 0.f))
            * glm::rotate(glm::mat4(1.f), m_rotation, glm::vec3(0.f, 0.f, 1.f))
            //* glm::scale(glm::mat4(1.f), glm::vec3(1.f, 1.f, 1.f)) // identity
            * glm::translate(glm::mat4(1.f), glm::vec3(-m_origin.x, -m_origin.y, 0.f));


        glm::vec2 position(glm::uninitialize);
        glm::vec2 scale(glm::uninitialize);
        float rotation;

        decompose(globalmtx, &position, &scale, &rotation);

        draw(m_texture, nullptr, 0xffffffff, position, glm::vec2(0.f, 0.f), glm::vec2(1.f, 1.f), rotation, m_depth);
        for (CompositeSprite* s : m_children)
        {
            s->render(globalmtx);
        }
    }
}