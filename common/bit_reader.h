#pragma once

#include <stdint.h>

namespace arena
{
    class BitReader
    {
    public:
        // Pointer to received data and how large it is
        BitReader(const void* data, uint32_t bytes);

        // reads DWORD (32 bits) and returns the result
        uint32_t readBits(uint32_t count);

        // Aligns the reader to byte boundary 
        // returns true if the actually read data is align, false if it is something else (user fault)
        bool readAlign();

        void readBytes(uint8_t* data, uint32_t bytes);

        uint32_t getAlignBits() const;

        uint32_t getBitsRead() const;

        uint32_t getBytesRead() const;

        uint32_t getBitsRemaining() const;

        uint32_t getBytesRemaining() const;

        uint32_t getTotalBits() const;

        uint32_t getTotalBytes() const;
    private:
        const uint32_t* m_data;
        uint64_t m_scratch;
        uint32_t m_numBits;
        uint32_t m_numBytes;

        uint32_t m_numWords;

        uint32_t m_bitsRead;
        uint32_t m_scratchBits;
        uint32_t m_wordIndex;
    };
}