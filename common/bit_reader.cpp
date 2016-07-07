#pragma once

#include "bit_reader.h"
#include "debug.h"
#include "uint32.h"

namespace arena
{
    BitReader::BitReader(const void* data, uint32_t bytes)
        : m_data((uint32_t*)data), m_numBytes(bytes),
          m_bitsRead(0), m_scratch(0), m_scratchBits(0), m_wordIndex(0),
          m_numWords((bytes + 3) / 4)

    {
        ARENA_ASSERT(data != nullptr, "Data can't be NULL");
        m_numBits = m_numBytes * 8;
    }

    uint32_t BitReader::readBits(uint32_t count)
    {
        ARENA_ASSERT(count > 0, "count must be more than 0");
        ARENA_ASSERT(count <= 32, "count must be less than 32");
        ARENA_ASSERT(m_bitsRead + count <= m_numBits, "Out of bounds");

        m_bitsRead += count;

        ARENA_ASSERT(m_scratchBits >= 0 && m_scratchBits <= 64, "Overflow");

        if (m_scratchBits < count)
        {
            ARENA_ASSERT(m_wordIndex < m_numWords, "Overflow");
            
            m_scratch |= uint64_t(networkToHost(m_data[m_wordIndex])) << m_scratchBits;
            m_scratchBits += 32;
            ++m_wordIndex;
        }

        ARENA_ASSERT(m_scratchBits >= count, "fucked up");

        
        uint32_t out = m_scratch & ((uint64_t(1) << count) - 1);
        m_scratch >>= count;
        m_scratchBits -= count;

        return out;
    }

    bool BitReader::readAlign()
    {
        const uint32_t remainderBits = m_bitsRead % 8;
        if (remainderBits != 0)
        {
            uint32_t value = readBits(8 - remainderBits);
            ARENA_ASSERT(m_bitsRead % 8 == 0, "Reader not aligned");
            // writer outputs 0 as align, if there is something else
            // it is user fault
            if (value != 0) 
            {
                return false;
            }
        }
        return true;
    }

    bool BitReader::wouldOverflow(uint32_t bits) const
    {
        return m_bitsRead + bits > m_numBits;
    }

    void BitReader::readBytes(uint8_t* data, uint32_t bytes)
    {
        ARENA_ASSERT(getAlignBits() == 0, "Reader not aligned to byte boundary");
        ARENA_ASSERT(m_bitsRead + bytes * 8 <= m_numBits, "Out of bounds");
        ARENA_ASSERT(0
            || (m_bitsRead % 32) == 0
            || (m_bitsRead % 32) == 8
            || (m_bitsRead % 32) == 16
            || (m_bitsRead % 32) == 24
            , "Reader not aligned to byte boundary");

        // How much we still have room in our current buffer
        uint32_t headBytes = (4 - (m_bitsRead % 32) / 8) % 4;

        // Do we have more space than we are going to read?
        if (headBytes > bytes)
        {
            // if so, we are going to just read least amount of bytes and early exit
            headBytes = bytes;
        }

        // read the bytes from our current buf
        for (uint32_t i = 0; i < headBytes; ++i)
        {
            data[i] = (uint8_t)readBits(8);
        }

        // early exit
        if (headBytes == bytes) return;

        ARENA_ASSERT(getAlignBits() == 0, "Reader not aligned");

        // how many words we still need to read
        int numWords = (bytes - headBytes) / 4;

        // read full words
        if (numWords > 0)
        {
            ARENA_ASSERT((m_bitsRead % 32) == 0, "Reader not aligned");
            // aligned read
            memcpy(data + headBytes, &m_data[m_wordIndex], numWords * 4);
            m_bitsRead += numWords * 32;
            m_wordIndex += numWords;
            // reset buffer so it aligns correctly
            m_scratchBits = 0;
        }

        ARENA_ASSERT(getAlignBits() == 0, "Reader not aligned");

        // if we started our reading not aligned, read left 0 to 3 bytes
        uint32_t tailStart = headBytes + numWords * 4;
        uint32_t tailBytes = bytes - tailStart;

        ARENA_ASSERT(tailBytes >= 0 && tailBytes < 4, "Bytes are fucked up");

        for (uint32_t i = 0; i < tailBytes; ++i)
        {
            data[tailStart + i] = (uint8_t)readBits(8);
        }

        ARENA_ASSERT(getAlignBits() == 0, "Reader not aligned");
        ARENA_ASSERT(headBytes + numWords * 4 + tailBytes == bytes, "Read truncated");
    }

    uint32_t BitReader::getAlignBits() const
    {
        return (8 - m_bitsRead % 8) % 8;
    }

    uint32_t BitReader::getBitsRead() const
    {
        return m_bitsRead;
    }

    uint32_t BitReader::getBytesRead() const
    {
        return m_wordIndex * 4;
    }

    uint32_t BitReader::getBitsRemaining() const
    {
        return m_numBits - m_bitsRead;
    }

    uint32_t BitReader::getBytesRemaining() const
    {
        return (m_numBits - m_bitsRead) / 8;
    }

    uint32_t BitReader::getTotalBits() const
    {
        return m_numBits;
    }

    uint32_t BitReader::getTotalBytes() const
    {
        return m_numBits / 8;
    }
}