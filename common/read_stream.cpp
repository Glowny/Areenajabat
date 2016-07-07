#pragma once
#include "read_stream.h"
#include "debug.h"
#include "uint32.h"
#include "error.h"

namespace arena
{
    ReadStream::ReadStream(const uint8_t* buffer, uint32_t bytes)
        : m_reader(buffer, bytes), m_bitsRead(0)
    {

    }

    bool ReadStream::serializeInteger(int32_t & value, int32_t min, int32_t max)
    {
        ARENA_ASSERT(min < max, "min needs to be less than max");
        const int bits = bitsRequired(min, max);

        if (m_reader.wouldOverflow(bits))
        {
            BX_ERROR_SET((&m_error), ARENA_ERROR_STREAM_OVERFLOW, "Stream would overflow");
            return false;
        }

        uint32_t unsignedValue = m_reader.readBits(bits);
        value = (int32_t)unsignedValue + min;
        m_bitsRead += bits;
        return true;
    }

    bool ReadStream::serializeBits(uint32_t & value, uint32_t bits)
    {
        ARENA_ASSERT(bits <= 32, "bits needs to be less than 32");
        
        if (m_reader.wouldOverflow(bits))
        {
            BX_ERROR_SET((&m_error), ARENA_ERROR_STREAM_OVERFLOW, "Stream would overflow");
            return false;
        }

        value = m_reader.readBits(bits);
        m_bitsRead += bits;

        return true;
    }

    bool ReadStream::serializeBytes(uint8_t * data, uint32_t bytes)
    {
        if (!serializeAlign())
        {
            return false;
        }

        if (m_reader.wouldOverflow(bytes * 8))
        {
            BX_ERROR_SET((&m_error), ARENA_ERROR_STREAM_OVERFLOW, "Stream would overflow");
            return false;
        }
        m_reader.readBytes(data, bytes);
        m_bitsRead += bytes * 8;
        return true;
    }

    bool ReadStream::serializeAlign()
    {
        const uint32_t align = m_reader.getAlignBits();
        if (m_reader.wouldOverflow(align))
        {
            BX_ERROR_SET((&m_error), ARENA_ERROR_STREAM_OVERFLOW, "Stream would overflow");
            return false;
        }

        if (!m_reader.readAlign())
        {
            return false;
        }
        m_bitsRead += align;
        return true;
    }

    uint32_t ReadStream::getBitsProcessed() const
    {
        return m_bitsRead;
    }

    uint32_t ReadStream::getBitsRemaining() const
    {
        return m_reader.getBitsRemaining();
    }

    uint32_t ReadStream::getBytesProcessed() const
    {
        return (m_bitsRead + 7) / 8;
    }

    uint32_t ReadStream::getBytesRead() const
    {
        return m_reader.getBytesRead();
    }
}