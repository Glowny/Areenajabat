#pragma once

#include "..\graphics\sprite.h"

#include "component.h"

namespace arena
{
	class SpriteRenderer final : public Component
	{
	public:	
		~SpriteRenderer() = default;

		glm::vec2& position();
		glm::vec2& origin();
		glm::vec2& scale();

		void rotate(const float32 amount);
		float32 rotation() const;

		TextureResource* const getTexture();
		void setTexture(TextureResource* const texture);
	private:
		Sprite m_sprite;
	};
}