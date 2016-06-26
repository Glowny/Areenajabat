#pragma once
#include <stdint.h>

namespace arena
{
    namespace color
    {
        inline uint32_t toABGR(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = 255)
        {
            return (alpha << 24) | (blue << 16) | (green << 8) | red;
        }

        inline uint32_t toABGR(uint32_t rgbaHex)
        {
            return
                (((rgbaHex >> 0) & 0xFF) << 24) | // alpha
                (((rgbaHex >> 8) & 0xFF) << 16) | // blue
                (((rgbaHex >> 16) & 0xFF) << 8) | // green
                (((rgbaHex >> 24) & 0xFF) << 0);   // red
        }
    }
}