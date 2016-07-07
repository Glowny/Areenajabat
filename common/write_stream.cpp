#include "write_stream.h"
#include "uint32.h"
namespace arena
{
    WriteStream::WriteStream(uint8_t* buffer, uint32_t bytes)
        : m_writer(buffer, bytes)
    {

    }

    bool WriteStream::serializeInteger(int32_t value, int32_t min, int32_t max)
    {
        ARENA_ASSERT(min < max, "Minimium needs to be smaller than max");
        ARENA_ASSERT(value >= min, "Value needs to be higher than min");
        ARENA_ASSERT(value <= max, "Value needs to be smaller than max");

        const int bits = bitsRequired(min, max);
        uint32_t unsignedValue = value - min;
        m_writer.writeBits(unsignedValue, bits);
        return true;
    }

    bool WriteStream::serializeBits(uint32_t value, uint32_t bits)
    {
        ARENA_ASSERT(bits <= 32, "Max 32 bits (got %d)", bits);
        m_writer.writeBits(value, bits);
        return true;
    }

    bool WriteStream::serializeBytes(const uint8_t* data, uint32_t bytes)
    {
        ARENA_ASSERT(data != nullptr, "Data can't be nullptr");
        // align to byte boundary
        m_writer.writeAlign();
        m_writer.writeBytes(data, bytes);
        return true;
    }

    bool WriteStream::serializeAlign()
    {
        m_writer.writeAlign();
        return true;
    }

    void WriteStream::flush()
    {
        m_writer.flushBits();
    }

    const uint8_t* WriteStream::getData() const
    {
        return m_writer.getData();
    }

    uint32_t WriteStream::getBytesProcessed() const
    {
        return m_writer.getBytesWritten();
    }

    uint32_t WriteStream::getBitsProcessed() const
    {
        return m_writer.getBitsWritten();
    }

    uint32_t WriteStream::getBitsRemaining() const
    {
        return (m_writer.getTotalBytes() * 8) - m_writer.getBitsWritten();
    }

    uint32_t WriteStream::getTotalBits() const
    {
        return m_writer.getTotalBytes() * 8;
    }

    uint32_t WriteStream::getTotalBytes() const
    {
        return m_writer.getTotalBytes();
    }
}
