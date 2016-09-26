#include "camera_manager.h"
#include "../sprite_renderer.h"
#include "../transform.h"
#include <bx/fpumath.h>
#include <bgfx\bgfx.h>
#include <glm/gtc/type_ptr.hpp>

namespace arena
{
	CameraManager::CameraManager() : ComponentManager()
	{
	}
	void CameraManager::setMousePointer(Entity* entity)
	{
		m_mousePointer = entity;
	}
	void CameraManager::setMouseState(MouseState mouseState)
	{
		m_mouseState = mouseState;
	}
	void CameraManager::onUpdate(const GameTime&)
	{
		auto it = begin();
		if (it == end())
			return;
		Camera camera = (*it)->m_camera;
		glm::vec2 playerPositionChange = glm::vec2(0, 0);

		if (m_character != nullptr)
		{
			Transform* playerTransform = (Transform* const)m_character->owner()->first(TYPEOF(Transform));
			playerPositionChange = playerTransform->m_position - m_lastPlayerPosition;
			m_lastPlayerPosition = playerTransform->m_position;
		};

		glm::vec2 movement = glm::vec2(m_mouseState.m_mrx, m_mouseState.m_mry);
		// Check that movement has been updated.
		if (movement.x == m_mouseLastMr.x && movement.y == m_mouseLastMr.y)
			movement = glm::ivec2(0, 0);
		m_mouseLastMr = glm::vec2(m_mouseState.m_mrx, m_mouseState.m_mry);

		glm::vec2 cameraPosition = glm::vec2(0, 0);
		if (m_mousePointer != nullptr)
		{
			Transform* mouseTransform = (Transform* const)m_mousePointer->first(TYPEOF(Transform));
			glm::vec2 newPosition = mouseTransform->m_position + movement + playerPositionChange;
			// Check that the mouse position is inside the map bounds.
			correctMousePosition(newPosition);
			mouseTransform->m_position = newPosition;
			// Check that the camera is inside the map bounds.
			cameraPosition = newPosition;
			correctCameraPosition(cameraPosition);

			// Set rotation of mouse pointer.
			SpriteRenderer* renderer = (SpriteRenderer* const)m_mousePointer->first(TYPEOF(SpriteRenderer));
			renderer->setRotation(m_character->m_gladiator.m_aimAngle + 1.5708f);
		}
		(*it)->m_camera.m_position = cameraPosition;
		(*it)->m_camera.calculate();
		// set views
		setViews();
	}

	void CameraManager::setBounds(glm::vec4 mouseBounds, glm::vec4 cameraBounds)
	{
		m_mouseBounds = mouseBounds;
		m_cameraBounds = cameraBounds;
	}

	void CameraManager::correctMousePosition(glm::vec2& position)
	{
		if (position.x < m_mouseBounds.x)
		{
			position.x = m_mouseBounds.x;
		}
		else if (position.x > m_mouseBounds.z)
		{
			position.x = m_mouseBounds.z;
		}
		if (position.y < m_mouseBounds.y)
		{
			position.y = m_mouseBounds.y;
		}
		else if (position.y > m_mouseBounds.w)
		{
			position.y = m_mouseBounds.w;
		}
	}
	void CameraManager::correctCameraPosition(glm::vec2& position)
	{
		if (position.x < m_cameraBounds.x)
		{
			position.x = m_cameraBounds.x;
		}
		else if (position.x > m_cameraBounds.z)
		{
			position.x = m_cameraBounds.z;
		}
		if (position.y < m_cameraBounds.y)
		{
			position.y = m_cameraBounds.y;
		}
		else if (position.y > m_cameraBounds.w)
		{
			position.y = m_cameraBounds.w;
		}
	}
	void CameraManager::setViews()
	{
		auto it = begin();
		if ((*it) == nullptr)
			return;
		Camera camera = (*it)->m_camera;
		float ortho[16];
		bx::mtxOrtho(ortho, 0.0f, float(camera.m_bounds.x), float(camera.m_bounds.y), 0.0f, 0.0f, 1000.0f);
		bgfx::setViewTransform(0, glm::value_ptr(camera.m_matrix), ortho);
		bgfx::setViewRect(0, 0, 0, uint16_t(camera.m_bounds.x), uint16_t(camera.m_bounds.y));

		bgfx::setViewTransform(1, glm::value_ptr(camera.m_matrix), ortho);
		bgfx::setViewRect(1, 0, 0, uint16_t(camera.m_bounds.x), uint16_t(camera.m_bounds.y));
	}


}