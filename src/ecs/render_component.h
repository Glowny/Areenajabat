#pragma once

#include "component.h"

#include "..\forward_declare.h"
#include "..\arena_types.h"
#include "..\graphics\layers.h"
#include "..\graphics\color.h"
#include "..\3rdparty\glm\vec2.hpp"

FORWARD_DECLARE_1(FORWARD_DECLARE_TYPE_CLASS, arena, Entity)

namespace arena
{
	class RenderComponent : public Component
	{
	SET_FRIENDS

	DEFINE_RTTI_SUB_TYPE(RenderComponent)

	public:
		uint32 getLayer() const;
		void setLayer(const uint32 layer);

		uint32 getColor() const;
		void setColor(const uint32 color);

		bool visible() const;
		void show();
		void hide();

		void anchor();
		void unanchor();
		bool isAnchored() const;

		glm::vec2& getOffset();

		virtual ~RenderComponent() = default;
	protected:
		RenderComponent() = default;
	private:
		uint32		m_layer		{ layers::Background };
		uint32		m_color		{ color::White		 };
		bool		m_visible	{ true };   
		bool		m_anchored	{ true };
		glm::vec2	m_offset;
	};
}
