#include "bit_writer.h"
#include "uint32.h"
namespace arena
{
    BitWriter::BitWriter(void* data, uint32_t bytes)
        : m_data((uint32_t*)data), m_numWords(bytes / 4),
        m_bitsWritten(0), m_wordIndex(0), m_scratch(0), m_scratchBits(0)
    {
        ARENA_ASSERT(data != nullptr, "Data can not be nullptr");
        ARENA_ASSERT(bytes % 4 == 0, "Preallocated buffer size needs be multiple of 4");
        m_numBits = m_numWords * 32; // we work in 32 bit space
    }

    void BitWriter::writeBits(uint32_t value, uint32_t bits)
    {
        ARENA_ASSERT(bits > 0, "Bits to write must be more than 0");
        ARENA_ASSERT(bits <= 32, "Bits to write must be less than 32");
        ARENA_ASSERT(m_bitsWritten + bits <= m_numBits,
            "The buffer would overflow: %d of %d bits in the buffer, trying to write %d bits",
            m_bitsWritten, m_numBits, bits);

        /*
        Example:
        Let value = 7, in binary 0000 0111, bits = 3
        (1 << 3)        = 0000 1000 = 8
        ((1 << 3) - 1)  = 0000 0111 = 7

        Let value = 8, in binary 0000 1000, bits(8) = 4
        (1 << 4)        = 0001 0000 = 16
        ((1 << 4) - 1)  = 0000 0001 = 15 = 0000 1111
        1000 & 1111 = 1000
        */
        value &= (uint64_t(1) << bits) - 1;

        /*
        Example:
        scratchbits = 0
        Write 7, 0111 << 0 
        0 |= 0111 = 00.....0111
        Write 8, 1000 << 3 
        00..0111 |= 0100 000 = 0...0100 0111
        */
        m_scratch |= uint64_t(value) << m_scratchBits;

        m_scratchBits += bits;

        // if we overflow
        if (m_scratchBits >= 32)
        {
            ARENA_ASSERT(m_wordIndex < m_numWords, "Out of space");
            // get low part of uint64_t 
            m_data[m_wordIndex] = hostToNetwork(uint32_t(m_scratch & 0xffffffff)); 
            // shift overflowed to low part
            m_scratch >>= 32;
            m_scratchBits -= 32;
            // change word
            ++m_wordIndex;
        }

        m_bitsWritten += bits;
    }
}