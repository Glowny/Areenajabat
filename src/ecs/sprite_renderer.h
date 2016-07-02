#pragma once

#include "..\forward_declare.h"
#include "..\graphics\sprite.h"
#include "render_component.h"

#include "..\rect.h"

FORWARD_DECLARE_1(FORWARD_DECLARE_TYPE_CLASS, arena, Entity)

namespace arena
{
	struct SpriteData final 
	{
	public:
		TextureResource* m_texture;
		glm::vec2		 m_position;
		glm::vec2		 m_origin;
		glm::vec2		 m_scale		{ 1.0f, 1.0f };
		Rectf			 m_src;
		float32			 m_rotation		{ 0.0f };

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
		Rectf& source();

		void rotate(const float32 amount);
		void setRotation(const float32 rotation);
		float32 getRotation() const;

		TextureResource* const getTexture();
		void setTexture(TextureResource* const texture, bool fitToArea = true);

		~SpriteRenderer() = default;
	private:
		SpriteData m_sprite;
	};
}