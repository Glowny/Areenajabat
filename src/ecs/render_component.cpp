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
		m_visible = true;
	}
	void RenderComponent::hide()
	{
		m_visible = false;
	}

	void RenderComponent::anchor()
	{
		m_anchored = true;
	}
	void RenderComponent::unanchor()
	{
		m_anchored = false;
	}
	bool RenderComponent::isAnchored() const 
	{
		return m_anchored;
	}

	glm::vec2& RenderComponent::getOffset()
	{
		return m_offset;
	}
}