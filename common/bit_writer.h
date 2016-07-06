#pragma once

#include <stdint.h>
#include "debug.h"

namespace arena
{
    // works in word level which means that the data needs to be multiple of four 
    class BitWriter
    {
    public:
        // Data must be preallocated, bytes must me multiple of four
        BitWriter(void* data, uint32_t bytes);

        // write amount of bits from value
        void writeBits(uint32_t value, uint32_t bits);
        // aligns the writer to byte, writes zero as padding
        void writeAlign();
        // assumes the writer is aligned to byte boundary
        void writeBytes(const uint8_t* data, uint32_t bytes);

        // returns how many bits we need to align to write to byte boundary
        uint32_t getAlignBits() const;

        void flushBits();
    private:
        // the buffer
        uint32_t* m_data;
        // scratch buffer, used to handle overflows
        // if we need 64 buffer this needs to be uint64_t * 2
        uint64_t m_scratch;
        // how many words
        uint32_t m_numWords;
        // how many bits total
        uint32_t m_numBits;
        // how many bits written in total
        uint32_t m_bitsWritten;
        // current word
        uint32_t m_wordIndex;
        // how many bits written in m_scratch
        uint32_t m_scratchBits;
    };
}