#pragma once

#include "..\graphics\sprite.h"

#include "render_component.h"

namespace arena
{
	struct SpriteData final 
	{
	public:
		TextureResource* m_texture;
		glm::vec2		 m_position;
		glm::vec2		 m_origin;
		glm::vec2		 m_scale;
		float32			 m_rotation;

		SpriteData()		 = default;
		~SpriteData()		 = default;
	};

	class SpriteRenderer final : public RenderComponent
	{
	public:	
		SpriteRenderer(Entity* const owner);

		glm::vec2& position();
		glm::vec2& origin();
		glm::vec2& scale();

		void rotate(const float32 amount);
		float32 rotation() const;

		TextureResource* const getTexture();
		void setTexture(TextureResource* const texture);

		~SpriteRenderer() = default;
	private:
		SpriteData m_sprite;
	};
}