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
		m_scale(1, 1),
		m_rotation(0.f),
		m_depth(0.f),
		m_hide(false)
    {

    }

    void CompositeSprite::render(uint8_t effect)
    {
		if (m_hide)
			return;
		render(glm::mat4(1.f), effect);
    }

    void CompositeSprite::render(const glm::mat4& parentmtx, uint8_t effect)
    {
		if (m_hide)
			return;
        glm::mat4 globalmtx = parentmtx
            * glm::translate(glm::mat4(1.f), glm::vec3(m_position.x, m_position.y, 0.f))
            * glm::rotate(glm::mat4(1.f), m_rotation, glm::vec3(0.f, 0.f, 1.f))
            * glm::scale(glm::mat4(1.f), glm::vec3(m_scale, 1.f))
            * glm::translate(glm::mat4(1.f), glm::vec3(-m_origin.x, -m_origin.y, 0.f));


        
        glm::vec2 scale(glm::uninitialize);
        float rotation;

        decompose(globalmtx, &m_globalPosition, &scale, &rotation);

        draw(m_texture, nullptr, 0xffffffff, m_globalPosition, glm::vec2(0.f, 0.f), scale, effect, rotation, m_depth);
        for (CompositeSprite* s : m_children)
        {
            s->render(globalmtx, effect);
        }
    }
}