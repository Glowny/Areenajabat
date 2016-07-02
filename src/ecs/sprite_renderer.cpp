#include "sprite_renderer.h"
#include "entity.h"

namespace arena
{
	SpriteRenderer::SpriteRenderer(Entity* const owner) : RenderComponent(owner)
	{
	}

	glm::vec2& SpriteRenderer::position()
	{
		return m_sprite.m_position;
	}
	glm::vec2& SpriteRenderer::origin()
	{
		return m_sprite.m_origin;
	}
	glm::vec2& SpriteRenderer::scale()
	{
		return m_sprite.m_scale;
	}
	glm::vec2& SpriteRenderer::sourcePosition()
	{
		return m_sprite.m_srcxy;
	}
	glm::vec2& SpriteRenderer::sourceSize()
	{
		return m_sprite.m_srcwh;
	}

	void SpriteRenderer::rotate(const float32 amount)
	{
		m_sprite.m_rotation += amount;
	}
	void SpriteRenderer::setRotation(const float32 rotation)
	{
		m_sprite.m_rotation = rotation;
	}
	float32 SpriteRenderer::getRotation() const
	{
		return m_sprite.m_rotation;
	}

	TextureResource* const SpriteRenderer::getTexture()
	{
		return m_sprite.m_texture;
	}
	void SpriteRenderer::setTexture(TextureResource* const texture)
	{
		m_sprite.m_texture = texture;
	}
}