#include "spritebatch.h"
#include <vector>
#include <algorithm>
#include "../res/resource_manager.h"
#include <glm/gtx/matrix_transform_2d.hpp>
#include "../res/texture_resource.h"
#include "vs_texture.bin.h"
#include "fs_texture.bin.h"

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

        s_texture = bgfx::createUniform("s_texColor", bgfx::UniformType::Int1);
        bgfx::ShaderHandle vsh = bgfx::createShader(bgfx::makeRef(vs_texture_dx11, sizeof(vs_texture_dx11)));
        bgfx::ShaderHandle fsh = bgfx::createShader(bgfx::makeRef(fs_texture_dx11, sizeof(fs_texture_dx11)));
        m_program = bgfx::createProgram(vsh, fsh, true); // destroy shaders
    }

    SpriteBatch::~SpriteBatch()
    {
        bgfx::destroyUniform(s_texture);
        bgfx::destroyIndexBuffer(m_ibh);
        bgfx::destroyProgram(m_program);
    }

    void SpriteBatch::draw(const TextureResource* texture, uint32_t color, const glm::vec2& position)
    {
        draw(texture, nullptr, color, position, glm::vec2(0, 0), glm::vec2(1, 1), SpriteEffects::None, 0.f, 0.f);
    }

    void SpriteBatch::draw(const TextureResource* texture, glm::vec4* src, uint32_t color, const glm::vec2& position, const glm::vec2& origin, const glm::vec2& scale, uint8_t effects, float angle, float depth)
    {
        if (m_spriteQueueCount >= m_spriteQueue.size())
        {
            uint32_t newSize = std::max(InitialQueueSize, (uint32_t)m_spriteQueue.size() * 2);
            m_spriteQueue.resize(newSize);
        }

        SpriteInfo* sprite = &m_spriteQueue[m_spriteQueueCount];

        float minu, maxu, minv, maxv;
        float width, height;
        if (src != NULL)
        {
            width = src->z;
            height = src->w;
            // x
            minu = src->x / (float)texture->width;
            maxu = (src->x + width) / (float)texture->width;
            // y
            // TODO OPENGL HOWTO
            minv = OriginBottomLeft ? 0 : src->y / (float)texture->height;
            maxv = OriginBottomLeft ? 0 : (src->y + height) / (float)texture->height;
        }
        else
        {
            minu = 0;
            maxu = 1.f;
            minv = OriginBottomLeft ? 1.0f : 0.f;
            maxv = OriginBottomLeft ? 0.0f : 1.f;
            width = texture->width;
            height = texture->height;
        }

        if ((effects & SpriteEffects::FlipVertically) != 0)
        {
            // bottom right = max
            // top left = min
            float temp = maxv;
            // bottom right y = top left y
            maxv = minv;
            minv = temp;
        }

        if ((effects & SpriteEffects::FlipHorizontally) != 0)
        {
            float temp = maxu;
            maxu = minu;
            minu = temp;
        }

        glm::vec2 pos(position + origin);
        glm::mat3 transform =
            glm::translate(glm::mat3(1.f), pos)
            * glm::rotate(glm::mat3(1.f), angle)
            * glm::scale(glm::mat3(1.f), scale)
            * glm::translate(glm::mat3(1.f), -pos);

        glm::vec3 points[4] =
        {
            transform * glm::vec3(position, 1.f),
            transform * glm::vec3(position.x + width, position.y, 1.f),
            transform * glm::vec3(position.x, position.y + height, 1.f),
            transform * glm::vec3(position.x + width, position.y + height, 1.f),
        };

        sprite->texture = texture;
        sprite->abgr = color;
        sprite->tl = glm::vec2(points[0].x, points[0].y);
        sprite->tr = glm::vec2(points[1].x, points[1].y);
        sprite->bl = glm::vec2(points[2].x, points[2].y);
        sprite->br = glm::vec2(points[3].x, points[3].y);
        sprite->u = glm::vec2(minu, maxu);
        sprite->v = glm::vec2(minv, maxv);
        sprite->depth = depth;
        ++m_spriteQueueCount;
    }
	void SpriteBatch::draw(const TextureResource* texture, Rectf& src, uint32_t color, const glm::vec2& position, const glm::vec2& origin, const glm::vec2& scale, uint8_t effects, float angle, float depth)
	{
		glm::vec4 v4src;
		v4src.x = src.x;
		v4src.y = src.y;
		v4src.z = src.w;
		v4src.w = src.h;

		draw(texture, &v4src, color, position, origin, scale, effects, angle, depth);
	}
	void SpriteBatch::draw(const TextureResource* texture, uint32_t color, const glm::vec2& position, const glm::vec2& origin, const glm::vec2& scale, uint8_t effects, float angle, float depth)
	{
		draw(texture, nullptr, color, position, origin, scale, effects, angle, depth);
	}

    void SpriteBatch::submit(uint8_t view)
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
                return x->depth < y->depth;
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
                        // TODO maybe hax
                        bgfx::setState(0
                            | BGFX_STATE_RGB_WRITE
                            | BGFX_STATE_ALPHA_WRITE
                            | BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA)
                        );
                        bgfx::setTexture(0, s_texture, batchTexture->handle);
                        bgfx::setVertexBuffer(&vb);
                        bgfx::setIndexBuffer(m_ibh, batchStart * 6, (pos - batchStart) * 6);
                        bgfx::submit(view, m_program);
                    }
                    batchTexture = texture;
                    batchStart = pos;
                }
            }
            // TODO maybe hax
            bgfx::setState(0
                | BGFX_STATE_RGB_WRITE
                | BGFX_STATE_ALPHA_WRITE
                | BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA)
            );
            bgfx::setTexture(0, s_texture, batchTexture->handle);
            bgfx::setVertexBuffer(&vb);
            bgfx::setIndexBuffer(m_ibh, batchStart * 6, (m_spriteQueueCount - batchStart) * 6);
            bgfx::submit(view, m_program);
        }

        m_spriteQueueCount = 0;
    }
}