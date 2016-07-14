#pragma once

#include "bit_reader.h"
#include <bx/error.h>

namespace arena
{
    class ReadStream
    {
    public:
        // hax for branching
        enum { IsWriting = 0 };
        enum { IsReading = 1 };

        ReadStream(const uint8_t* buffer, uint32_t bytes);

        // deserialize integer to value
        // return true if succeeded
        bool serializeInteger(int32_t& value, int32_t min, int32_t max);

        // deserialize bits to value
        // return true if succeeded
        bool serializeBits(uint32_t& value, uint32_t bits);

        // deserialize bytes to data
        // return true if succeeded
        bool serializeBytes(uint8_t* data, uint32_t bytes);

        // align reader to boundary
        // returns true if read data was align
        bool serializeAlign();

        uint32_t getBitsProcessed() const;

        uint32_t getBitsRemaining() const;

        uint32_t getBytesProcessed() const;

        uint32_t getBytesRead() const;

        bx::Error m_error;
    private:
        BitReader m_reader;
        uint32_t m_bitsRead;
        
    };
}