#pragma once

#include "../../mem/pool_allocator.h"
#include "component_manager.h"
#include "../camera_component.h"
#include "../character_component.h"
#include "../../singleton.h"
#include "../entity.h"
namespace arena
{
	class CameraManager final : public ComponentManager<CameraComponent>
	{
	public:
		void setMousePointer(Entity* entity);
		virtual void onUpdate(const GameTime&);
		void setBounds(glm::vec4 mouseBounds, glm::vec4 cameraBounds);
		void setMouseState(MouseState mouseState);
		// For weapon rotation.
		CharacterComponent* m_character;
		Entity* m_mousePointer;
		MAKE_SINGLETON(CameraManager)
	private:
		// Hard coded default values on current map values, use setBounds().
		glm::vec4 m_mouseBounds{ 0, 0, 7680, 2160 };
		glm::vec4 m_cameraBounds{ 1080, 540, 6720, 1620 };
		glm::vec2 m_lastPlayerPosition{ 0, 0 };
		glm::ivec2 m_mouseLastMr{ 0, 0 };
		MouseState m_mouseState;

		void correctMousePosition(glm::vec2& position);
		void correctCameraPosition(glm::vec2& position);
		void setViews();
	};
}