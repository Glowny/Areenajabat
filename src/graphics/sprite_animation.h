#pragma once

#include <stdint.h>
#include <vector>


namespace arena
{
    class TextureResource;

    struct Frame
    {
        uint16_t x;
        uint16_t y;
        uint32_t time;
    };

    class SpriteAnimation
    {
    public:
        SpriteAnimation(const TextureResource* spritesheet, 
            uint16_t frameWidth, uint16_t frameHeight,
            uint16_t rows, uint16_t columns, uint32_t frameCount);

        void update(float dt);
    public:
        const TextureResource* m_spritesheet;
        uint32_t m_currentFrame;

        uint16_t m_frameWidth;
        uint16_t m_frameHeight;

        uint32_t m_elapsed;
        uint32_t m_totalTime;
        std::vector<Frame> m_frames;
    };
}