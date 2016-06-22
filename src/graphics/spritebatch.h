#pragma once

#include <bgfx/bgfx.h>
#include <glm/glm.hpp>
#include <stdint.h>
#include <vector>

namespace arena
{
    class TextureResource;

    struct PosUvColorVertex
    {
        float m_x;
        float m_y;
        float m_u;
        float m_v;
        uint32_t m_abgr;
    };

    struct SpriteInfo
    {
        glm::vec2 tl;
        glm::vec2 tr;
        glm::vec2 bl;
        glm::vec2 br;
        uint32_t abgr;
        glm::vec2 u;
        glm::vec2 v;
        const TextureResource* texture;
    };

    class SpriteBatch
    {
    public:
        static const uint32_t MaxBatchSize = 2048;
        static const uint32_t InitialQueueSize = 64;

        SpriteBatch();
        ~SpriteBatch();

        void submit(uint8_t view);
        void draw(const TextureResource* texture, const glm::vec2& position);

        SpriteBatch(SpriteBatch const&) = delete;
        SpriteBatch& operator=(SpriteBatch const&) = delete;

    private:
        bgfx::VertexDecl m_decl;
        bgfx::IndexBufferHandle m_ibh;

        std::vector<SpriteInfo> m_spriteQueue;

        std::vector<SpriteInfo const*> m_sortedSprites;

        uint32_t m_spriteQueueSize;
        uint32_t m_spriteQueueCount;

        bgfx::UniformHandle s_texture;
        bgfx::ProgramHandle m_program;
    };
}