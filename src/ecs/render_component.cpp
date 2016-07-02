#include "render_component.h"

namespace arena
{
	RenderComponent::RenderComponent(Entity* const owner) : Component(owner)
	{
	}

	uint32 RenderComponent::getLayer() const
	{
		return m_layer;
	}
	void RenderComponent::setLayer(const uint32 layer)
	{
		m_layer = layer;
	}

	uint32 RenderComponent::getColor() const
	{
		return m_color;
	}
	void RenderComponent::setColor(const uint32 color)
	{
		m_color = color;
	}

	bool RenderComponent::visible() const
	{
		return m_visible;
	}
	void RenderComponent::show()
	{
	}
	void RenderComponent::hide()
	{
	}
}