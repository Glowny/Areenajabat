#include "sprite_animation.h"
#include "../res/texture_resource.h"

namespace arena
{
    SpriteAnimation::SpriteAnimation(const TextureResource* texture, 
        uint16_t width, uint16_t height, uint32_t frameCount)
        : m_spritesheet(texture), m_frameWidth(width), m_frameHeight(height),
        m_currentFrame(0), m_elapsed(0), m_totalTime(0)
    {
        m_frames.resize(frameCount);
        memset(m_frames.data(), 0, sizeof(Frame) * frameCount);

        uint16_t columns = texture->width / m_frameWidth;

        uint32_t time = 0;
        uint32_t delta = 100;
        for (uint16_t i = 0; i < frameCount; ++i)
        {
            m_frames[i] = (Frame{
                uint16_t((i % columns) * m_frameWidth),
                uint16_t((i / columns) * m_frameHeight),
                time += delta
            });
            m_totalTime += delta;
        }
    }



    void SpriteAnimation::update(float dt)
    {
        m_elapsed += uint32_t(dt * 1000);

        if (m_elapsed >= m_frames[m_currentFrame].time)
        {
            m_currentFrame = (m_currentFrame + 1) % m_frames.size();
            m_elapsed = m_elapsed % m_totalTime;
        }
    }
}