#pragma once

#include "component.h"

#include "..\forward_declare.h"
#include "..\arena_types.h"
#include "..\graphics\layers.h"
#include "..\graphics\color.h"

FORWARD_DECLARE_1(FORWARD_DECLARE_TYPE_CLASS, arena, Entity)

namespace arena
{
	class RenderComponent : public Component
	{
	public:
		uint32 getLayer() const;
		void setLayer(const uint32 layer);

		uint32 getColor() const;
		void setColor(const uint32 color);

		bool visible() const;
		void show();
		void hide();

		virtual ~RenderComponent() = default;
	protected:
		RenderComponent(Entity* const owner);
	private:
		uint32 m_layer	 { layers::Background };
		uint32 m_color	 { color::White		 };
		bool   m_visible { true };   
	};
}
