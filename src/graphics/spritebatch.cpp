#include "spritebatch.h"
#include <vector>
#include <algorithm>
#include "../res/resource_manager.h"
#include <glm/gtx/matrix_transform_2d.hpp>

namespace arena
{

    static const bool OriginBottomLeft = BX_PLATFORM_WINDOWS != 0 ? false : true;

    SpriteBatch::SpriteBatch()
        : m_spriteQueueSize(0),
          m_spriteQueueCount(0)
    {
        const uint32_t indicesCount = MaxBatchSize * 6;

        std::vector<uint16_t> indices;
        indices.reserve(indicesCount);

        for (uint32_t i = 0; i < indicesCount; i += 4)
        {
            indices.push_back(i);
            indices.push_back(i + 1);
            indices.push_back(i + 2);

            indices.push_back(i + 1);
            indices.push_back(i + 3);
            indices.push_back(i + 2);
        }

        m_ibh = bgfx::createIndexBuffer(bgfx::copy(indices.data(), indicesCount * sizeof(uint16_t)));

        m_decl
            .begin()
            .add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float)
            .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
            .end();

        // TODO get this some other way
        s_texture = bgfx::createUniform("s_texColor", bgfx::UniformType::Int1);
    }

    SpriteBatch::~SpriteBatch()
    {
        bgfx::destroyUniform(s_texture);
    }

    void SpriteBatch::draw(const TextureResource* texture, const glm::vec2& position)
    {
        if (m_spriteQueueCount >= m_spriteQueue.size())
        {
            uint32_t newSize = std::max(InitialQueueSize, (uint32_t)m_spriteQueue.size() * 2);
            m_spriteQueue.resize(newSize);
        }

        SpriteInfo* sprite = &m_spriteQueue[m_spriteQueueCount];

        glm::vec2 origin;
        glm::vec2 scale(1, 1);
        uint32_t agbr = 0xFFFFFFFF;
        float angle = 0;
        glm::vec2 pos(position + origin);
        glm::mat3 transform =
            glm::translate(glm::mat3(1.f), pos)
            * glm::scale(glm::mat3(1.f), scale)
            * glm::rotate(glm::mat3(1.f), angle)
            * glm::translate(glm::mat3(1.f), -pos);

        glm::vec3 points[4] =
        {
            transform * glm::vec3(position, 1.f),
            transform * glm::vec3(position.x + texture->width, position.y, 1.f),
            transform * glm::vec3(position.x, position.y + texture->height, 1.f),
            transform * glm::vec3(position.x + texture->width, position.y + texture->height, 1.f),
        };

                // TODO move 
        //float m_halfTexel = 0.0f;
        const float texelHalfW = 0; //m_halfTexel / widthf;
        const float texelHalfH = 0; //m_halfTexel / heightf;
        const float minu = texelHalfW;
        const float maxu = 1.0f - texelHalfW;
        const float minv = OriginBottomLeft ? texelHalfH + 1.0f : texelHalfH;
        const float maxv = OriginBottomLeft ? texelHalfH : texelHalfH + 1.0f;

        sprite->texture = texture;
        sprite->abgr = agbr;
        sprite->tl = glm::vec2(points[0].x, points[0].y);
        sprite->tr = glm::vec2(points[1].x, points[1].y);
        sprite->bl = glm::vec2(points[2].x, points[2].y);
        sprite->br = glm::vec2(points[3].x, points[3].y);
        sprite->u = glm::vec2(minu, maxu);
        sprite->v = glm::vec2(minv, maxv);

        ++m_spriteQueueCount;
    }

    void SpriteBatch::submit(uint8_t view, bgfx::ProgramHandle program)
    {

        if (m_sortedSprites.size() < m_spriteQueueCount)
        {
            uint32_t previousSize = (uint32_t)m_sortedSprites.size();
            m_sortedSprites.resize(m_spriteQueueCount);
            for (size_t i = previousSize; i < m_spriteQueueCount; ++i)
            {
                m_sortedSprites[i] = &m_spriteQueue[i];
            }

            std::sort(
                std::begin(m_sortedSprites),
                std::begin(m_sortedSprites) + m_spriteQueueCount,
                [](const SpriteInfo* x, const SpriteInfo* y)
            {
                return x->texture->handle.idx < y->texture->handle.idx;
            });
        }
        
        if (bgfx::checkAvailTransientVertexBuffer(m_spriteQueueCount * 4, m_decl))
        {
            bgfx::TransientVertexBuffer vb;
            bgfx::allocTransientVertexBuffer(&vb, 4 * m_spriteQueueCount, m_decl);
            PosUvColorVertex* vertex = (PosUvColorVertex*)vb.data;

            for (uint32_t i = 0, v=0; i < m_spriteQueueCount; ++i, v += 4)
            {
                const SpriteInfo* s = m_sortedSprites[i];
                vertex[v + 0] = PosUvColorVertex{ s->tl.x, s->tl.y, s->u.x, s->v.x, s->abgr };
                vertex[v + 1] = PosUvColorVertex{ s->tr.x, s->tr.y, s->u.y, s->v.x, s->abgr };
                vertex[v + 2] = PosUvColorVertex{ s->bl.x, s->bl.y, s->u.x, s->v.y, s->abgr };
                vertex[v + 3] = PosUvColorVertex{ s->br.x, s->br.y, s->u.y, s->v.y, s->abgr };
            }

            const TextureResource* batchTexture = nullptr;
            uint32_t batchStart = 0;

            for (uint32_t pos = 0; pos < m_spriteQueueCount; ++pos)
            {
                const TextureResource* texture = m_sortedSprites[pos]->texture;

                if (texture != batchTexture)
                {
                    if (pos > batchStart)
                    {
                        // TODO FIX
                        bgfx::setState(0
                            | BGFX_STATE_RGB_WRITE
                            | BGFX_STATE_ALPHA_WRITE
                            | BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA)
                        );
                        bgfx::setTexture(0, s_texture, batchTexture->handle);
                        bgfx::setVertexBuffer(&vb);
                        bgfx::setIndexBuffer(m_ibh, batchStart * 6, (pos - batchStart) * 6);
                        bgfx::submit(view, program);
                    }
                    batchTexture = texture;
                    batchStart = pos;
                }
            }
            // TODO FIX
            bgfx::setState(0
                | BGFX_STATE_RGB_WRITE
                | BGFX_STATE_ALPHA_WRITE
                | BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA)
            );
            bgfx::setTexture(0, s_texture, batchTexture->handle);
            bgfx::setVertexBuffer(&vb);
            bgfx::setIndexBuffer(m_ibh, batchStart * 6, (m_spriteQueueCount - batchStart) * 6);
            bgfx::submit(view, program);
        }

        m_spriteQueueCount = 0;
    }
}