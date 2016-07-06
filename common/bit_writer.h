#pragma once

#include <stdint.h>

namespace arena
{
    // works in word level which means that the data needs to be multiple of four 
    class BitWriter
    {
    public:
        // Data must be preallocated, bytes must me multiple of four
        BitWriter(void* data, uint32_t bytes) :
            m_data((uint32_t*)data),
            m_numWords(bytes / 4)
        {

        }


    private:
        uint32_t* m_data;
        uint32_t m_numWords;
    };
}