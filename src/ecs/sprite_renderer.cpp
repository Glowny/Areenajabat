#include "sprite_renderer.h"
#include "entity.h"

#include "..\res\texture_resource.h"

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
	Rectf& SpriteRenderer::source()
	{
		return m_sprite.m_src;
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
	void SpriteRenderer::setTexture(TextureResource* const texture, bool fitToArea)
	{
		if (texture == nullptr) return;

		m_sprite.m_texture = texture;

		if (fitToArea) 
		{
			Rectf& src = m_sprite.m_src;
			src.x = 0.0f;
			src.y = 0.0f;
			src.h = float32(texture->width);
			src.w = float32(texture->height);
		}
	}
}