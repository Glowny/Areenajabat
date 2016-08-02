#include "sprite_renderer.h"
#include "entity.h"

#include "../res/texture_resource.h"

#include "../rtti/rtti_define.h"

namespace arena
{
	REGISTER_RTTI_SUB_TYPE(SpriteRenderer)

	glm::vec2& SpriteRenderer::getPosition()
	{
		return m_sprite.m_position;
	}
	glm::vec2& SpriteRenderer::getOrigin()
	{
		return m_sprite.m_origin;
	}
	glm::vec2& SpriteRenderer::getScale()
	{
		return m_sprite.m_scale;
	}
	Rectf& SpriteRenderer::getSource()
	{
		return m_sprite.m_src;
	}

	uint8 SpriteRenderer::getEffects() const
	{
		return m_sprite.m_effects;
	}
	void SpriteRenderer::setEffects(const uint8 effects)
	{
		m_sprite.m_effects = effects;
	}

	void SpriteRenderer::setSize(const float32 width, const float32 height)
	{
		m_sprite.m_size.x = width;
		m_sprite.m_size.y = height;

		if (m_sprite.m_texture != nullptr)
		{
			m_sprite.m_scale.x = width / float32(m_sprite.m_texture->width);
			m_sprite.m_scale.y = height / float32(m_sprite.m_texture->height);
		}
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
		if (texture == nullptr) return;

		m_sprite.m_texture = texture;

		const float32 width = float32(texture->width);
		const float32 height = float32(texture->height);

		m_sprite.m_src.x = 0.0f;
		m_sprite.m_src.y = 0.0f;
		m_sprite.m_src.h = height;
		m_sprite.m_src.w = width;

		if (int32(m_sprite.m_size.x) == 0 && int32(m_sprite.m_size.y) == 0)
		{
			m_sprite.m_size.x = width;
			m_sprite.m_size.y = height;
		}

		m_sprite.m_scale.x = m_sprite.m_size.x / float32(m_sprite.m_texture->width);
		m_sprite.m_scale.y = m_sprite.m_size.y / float32(m_sprite.m_texture->height);
	}
}