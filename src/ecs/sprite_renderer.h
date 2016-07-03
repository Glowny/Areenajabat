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
		TextureResource* m_texture		{ nullptr };
		glm::vec2		 m_position;
		glm::vec2		 m_origin;
		glm::vec2		 m_scale		{ 1.0f, 1.0f };
		glm::vec2		 m_size;
		Rectf			 m_src;
		float32			 m_rotation		{ 0.0f };
		uint8			 m_effects		{ NULL };

		SpriteData()		 = default;
		~SpriteData()		 = default;
	};

	class SpriteRenderer final : public RenderComponent
	{
	template<typename T>
	friend class ComponentManager;

	DEFINE_RTTI_SUB_TYPE(SpriteRenderer)
	
	public:	
		glm::vec2& getPosition();
		glm::vec2& getOrigin();
		glm::vec2& getScale();
		Rectf& getSource();

		uint8 getEffects() const;
		void setEffects(const uint8 effects);

		void setSize(const float32 width, const float32 height);

		void rotate(const float32 amount);
		void setRotation(const float32 rotation);
		float32 getRotation() const;

		TextureResource* const getTexture();
		void setTexture(TextureResource* const texture);

		~SpriteRenderer() = default;
	protected:
		SpriteRenderer() = default;
	private:
		SpriteData m_sprite;
	};
}