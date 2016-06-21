#include "spritebatch.h"
#include <vector>
#include <algorithm>
#include "../res/resource_manager.h"
#include <glm/gtx/matrix_transform_2d.hpp>

namespace arena
{

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

        m_vertices.resize(InitialQueueSize);
    }

    SpriteBatch::~SpriteBatch()
    {
        
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

        sprite->texture = texture;
        sprite->abgr = agbr;
        sprite->tl = glm::vec2(points[0].x, points[0].y);
        sprite->tr = glm::vec2(points[1].x, points[1].y);
        sprite->bl = glm::vec2(points[2].x, points[2].y);
        sprite->br = glm::vec2(points[3].x, points[3].y);

        ++m_spriteQueueCount;
    }

    void SpriteBatch::submit(uint8_t view, bgfx::ProgramHandle program)
    {
        bgfx::submit(view, program);
        (void)view;
        (void)program;
    }
}