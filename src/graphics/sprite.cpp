#include "sprite.h"
#include <bgfx/bgfx.h>
#include <glm/gtx/matrix_transform_2d.hpp>
#include "../res/resource_manager.h"

namespace arena
{
    void Sprite::submit(uint8_t view, bgfx::ProgramHandle program, uint64_t state) const
    {
        (void)view;
        (void)program;
        (void)state;
        glm::vec2 pos(m_position + m_origin);
        glm::mat3 transform =
            glm::translate(glm::mat3(1.f), pos)
            * glm::scale(glm::mat3(1.f), m_scale)
            * glm::rotate(glm::mat3(1.f), m_angle)
            * glm::translate(glm::mat3(1.f), -pos);

        glm::vec3 points[4] =
        {
            transform * glm::vec3(m_position, 1.f),
            transform * glm::vec3(m_position.x + m_texture->width, m_position.y, 1.f),
            transform * glm::vec3(m_position.x + m_texture->width, m_position.y + m_texture->height, 1.f),
            transform * glm::vec3(m_position.x, m_position.y + m_texture->height, 1.f),
        };

      /*  if (bgfx::checkAvailTransientVertexBuffer(6, PosUvColorVertex::ms_decl))
        {
            bgfx::TransientVertexBuffer vb;
            bgfx::allocTransientVertexBuffer(&vb, 6, PosUvColorVertex::ms_decl);
            PosUvColorVertex* vertex = (PosUvColorVertex*)vb.data;

            const float widthf = float(m_texture->width);
            const float heightf = float(m_texture->height);

            float m_halfTexel = 0.0f;

            const float texelHalfW = m_halfTexel / widthf;
            const float texelHalfH = m_halfTexel / heightf;
            const float minu = texelHalfW;
            const float maxu = 1.0f - texelHalfW;
            const float minv = OriginBottomLeft ? texelHalfH + 1.0f : texelHalfH;
            const float maxv = OriginBottomLeft ? texelHalfH : texelHalfH + 1.0f;

            vertex[0].m_x = points[0].x; // left
            vertex[0].m_y = points[0].y;
            vertex[0].m_u = minu;
            vertex[0].m_v = minv;

            vertex[1].m_x = points[1].x; // right
            vertex[1].m_y = points[1].y;
            vertex[1].m_u = maxu;
            vertex[1].m_v = minv;

            vertex[2].m_x = points[2].x;
            vertex[2].m_y = points[2].y;
            vertex[2].m_u = maxu;
            vertex[2].m_v = maxv;

            vertex[3].m_x = points[2].x;
            vertex[3].m_y = points[2].y;
            vertex[3].m_u = maxu;
            vertex[3].m_v = maxv;

            vertex[4].m_x = points[3].x;
            vertex[4].m_y = points[3].y;
            vertex[4].m_u = minu;
            vertex[4].m_v = maxv;

            vertex[5].m_x = points[0].x;
            vertex[5].m_y = points[0].y;
            vertex[5].m_u = minu;
            vertex[5].m_v = minv;

            vertex[0].m_abgr = m_abgr;
            vertex[1].m_abgr = m_abgr;
            vertex[2].m_abgr = m_abgr;
            vertex[3].m_abgr = m_abgr;
            vertex[4].m_abgr = m_abgr;
            vertex[5].m_abgr = m_abgr;

            bgfx::setVertexBuffer(&vb);
        }*/
    
    }
}