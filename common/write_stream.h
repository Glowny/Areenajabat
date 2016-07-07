#pragma once
#include <bx/error.h>
#include "bit_writer.h"

BX_ERROR_RESULT(ARENA_ERROR_STREAM_OVERFLOW, BX_MAKEFOURCC('N', 'E', 'T', 0));
BX_ERROR_RESULT(ARENA_ERROR_STREAM_ABORTED, BX_MAKEFOURCC('N', 'E', 'T', 1));

namespace arena
{
    class WriteStream
    {
    public:
        // hax for branching
        enum { IsWriting = 1 };
        enum { IsReading = 0 };

        WriteStream(uint8_t* buffer, uint32_t bytes);

        bool serializeInteger(int32_t value, int32_t min, int32_t max);

        bool serializeBits(uint32_t value, uint32_t bits);
        
        bool serializeBytes(const uint8_t* data, uint32_t bytes);

        bool serializeAlign();

        void flush();

        const uint8_t* getData() const;

        uint32_t getBytesProcessed() const;

        uint32_t getBitsProcessed() const;

        uint32_t getBitsRemaining() const;

        uint32_t getTotalBits() const;

        uint32_t getTotalBytes() const;

    private:
        bx::Error m_error;
        BitWriter m_writer;
    };
}