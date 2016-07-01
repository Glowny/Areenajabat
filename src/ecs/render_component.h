#pragma once

#include "component.h"

#include "..\arena_types.h"
#include "..\graphics\layers.h"
#include "..\graphics\color.h"

namespace arena
{
	class RenderComponent : public Component
	{
	public:
		uint32 getLayer() const;
		void setLayer(const uint32 layer);

		uint32 getColor() const;
		void setColor(const uint32 color);

		virtual ~RenderComponent() = default;
	private:
		uint32 m_layer	{ layers::Background };
		uint32 m_color	{ color::White		 };
	};
}
