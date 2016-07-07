#pragma once
#include "read_stream.h"
#include "debug.h"
#include "uint32.h"

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
            //m_error =
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
        (void)value;
        return false;
    }

    bool ReadStream::serializeBytes(uint8_t * data, uint32_t bytes)
    {
        (void)data;
        (void)bytes;
        if (!serializeAlign())
        {
            return false;
        }
        return false;
    }

    bool ReadStream::serializeAlign()
    {
        return false;
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