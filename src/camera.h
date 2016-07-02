#pragma once

#include <glm/glm.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace arena
{
    struct Camera
    {
        glm::vec2 m_position;
        float m_zoom;
        float m_angle;
        glm::vec2 m_bounds;
        glm::mat4 m_matrix;

        Camera(float width, float height)
            :
            m_position(0.f, 0.f),
            m_zoom(1.f),
            m_angle(0.f),
            m_bounds(width, height),
            m_matrix(1.f)
        {

        }

		Camera() : Camera(0.0f, 0.0f)
		{
		}

        void calculate()
        {
            m_matrix =
                glm::translate(glm::mat4(1.f), glm::vec3(m_bounds * 0.5f, 0.f)) *
                glm::scale(glm::mat4(1.f), glm::vec3(m_zoom)) *
                glm::rotate(glm::mat4(1.f), m_angle, glm::vec3(0, 0, 1.f)) *
                glm::translate(glm::mat4(1.f), glm::vec3(-m_position, 0.f));
        }
    };
}