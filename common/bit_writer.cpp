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

    void BitWriter::writeAlign()
    {
        const int remainderBits = m_bitsWritten % 8;
        if (remainderBits != 0)
        {
            uint32_t zero = 0;
            writeBits(zero, 8 - remainderBits);
            ARENA_ASSERT((m_bitsWritten % 8) == 0, "Write not aligned");
        }
    }

    uint32_t BitWriter::getAlignBits() const
    {
        // let m_bitsWritten = 65
        // m_bitsWritten % 8 = 1
        // (8 - 1) = 7
        // 7 % 8 = 7

        // let m_bitsWritten = 64
        // m_bitsWritten % 8 = 
        // 8 - 0 = 8
        // 8 % 8 = 0, aligned
        return (8 - (m_bitsWritten % 8) % 8);
    }

    void BitWriter::writeBytes(const uint8_t* data, uint32_t bytes)
    {
        ARENA_ASSERT(getAlignBits() == 0, "Writer must be aligned");
        ARENA_ASSERT(m_bitsWritten + (bytes * 8) <= m_numBits, 
            "Out of bounds, trying to write %d bits, %d of %d",
            bytes * 8, m_bitsWritten, m_numBits);
        ARENA_ASSERT(0
            || (m_bitsWritten % 32) == 0
            || (m_bitsWritten % 32) == 8
            || (m_bitsWritten % 32) == 16
            || (m_bitsWritten % 32) == 24
        , "Writer not aligned");

        /*
        Okay, lets say we have wrote 1 byte (8 bits), there are still 
        room left for 3 bytes (24 bits), so this is going to evaluate as 3
        */
        uint32_t headbytes = (4 - (m_bitsWritten % 32) / 8) % 4;

        // if there is still more room in buffer than we are going to write
        // we can exit early
        if (headbytes > bytes)
        {
            headbytes = bytes;
        }
        // flush the first bytes to current buffer
        for (uint32_t i = 0; i < headbytes; ++i)
        {
            writeBits(data[i], 8);
        }

        // early exit
        if (headbytes == bytes) return;

        // if we are here, we have filled the remaining buffer,
        // so ask for new 4 byte buffer
        flushBits();

        // it should be aligned correctly because it's just switched
        ARENA_ASSERT(getAlignBits() == 0, "Writer not aligned");

        // now lets say we have wrote 3 bytes of 4
        // so 4-3
        uint32_t numWords = (bytes - headbytes) / 4;

        if (numWords > 0)
        {
            ARENA_ASSERT((m_bitsWritten % 32) == 0, "Writer not aligned");
            // flushBits changeds the wordIndex so we are writing to aligned buffer
            memcpy(&m_data[m_wordIndex], data + headbytes, numWords * 4);
            m_bitsWritten += numWords * 32;
            m_wordIndex += numWords;
            // reset the buffer
            m_scratch = 0;
        }

        // writer should be aligned at this point
        ARENA_ASSERT(getAlignBits() == 0, "Writer not aligned");

        // if we filled our buffer with 3 bytes, this is going to give us the
        // last byte to write
        uint32_t tailStart = headbytes + numWords * 4;
        uint32_t tailBytes = bytes - tailStart;

        // it can't be over 4
        ARENA_ASSERT(tailBytes >= 0 && tailBytes < 4, "Bytes are fucked up");

        for (uint32_t i = 0; i < tailBytes; ++i)
        {
            writeBits(data[tailStart + i], 8);
        }
        // it must be aligned if we started with aligned buffer
        ARENA_ASSERT(getAlignBits() == 0, "Writer not aligned");

        ARENA_ASSERT(headbytes + numWords * 4 + tailBytes == bytes,
            "Write truncated: wrote %d bytes out of %d",
            headbytes + numWords * 4 + tailBytes,
            bytes);
    }

    void BitWriter::flushBits()
    {
        if (m_scratchBits != 0)
        {
            ARENA_ASSERT(m_wordIndex < m_numWords, "Out of space");
            m_data[m_wordIndex] = hostToNetwork(uint32_t(m_scratch & 0xffffffff));
            m_scratch >>= 32;
            m_scratchBits -= 32;
            ++m_wordIndex;
        }
    }

    uint32_t BitWriter::getBitsWritten() const
    {
        return m_bitsWritten;
    }

    uint32_t BitWriter::getBitsAvailable() const
    {
        return m_numBits - m_bitsWritten;
    }

    uint32_t BitWriter::getBytesWritten() const
    {
        return (m_bitsWritten + 7) / 8;
    }

    uint32_t BitWriter::getTotalBytes() const
    {
        return m_numWords * 4;
    }

    const uint8_t* BitWriter::getData() const
    {
        return (uint8_t*)m_data;
    }
}