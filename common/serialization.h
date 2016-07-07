#pragma once
#include "debug.h" // ARENA_ASSERT

// deserialization & serialization
#define serialize_bits(stream, value, bits)                                                             \
    do                                                                                                  \
    {                                                                                                   \
        ARENA_ASSERT(bits > 0, "Bits needs to be more than 0");                                         \
        ARENA_ASSERT(bits <= 32, "Bits cant be more than 32");                                          \
        uint32_t uv;                                                                                    \
        if (Stream::IsWriting)                                                                          \
        {                                                                                               \
            uv = (uint32_t)value;                                                                       \
        }                                                                                               \
        if (!stream.serializeBits(uv, bits))                                                            \
        {                                                                                               \
            return false;                                                                               \
        }                                                                                               \
        if (Stream::IsReading)                                                                          \
        {                                                                                               \
            value = uv;                                                                                 \
        }                                                                                               \
    } while (0)

namespace arena
{
    namespace detail
    {
        template <typename Stream>
        bool serializeFloat(Stream& stream, float& value)
        {
            uint32_t intvalue;
            if (Stream::IsWriting)
            {
                memcpy(&intvalue, &value, sizeof(uint32_t));
            }
            bool result = stream.serializeBits(intvalue, 32);

            if (Stream::IsReading)
            {
                memcpy(&value, &intvalue, sizeof(float));
            }

            return result;
        }

        template <typename Stream>
        bool serializeUint64(Stream& stream, uint64_t& value)
        {
            uint32_t hi, lo;
            if (Stream::IsWriting)
            {
                lo = value & 0xffffffff;
                hi = value >> 32;
            }
            serialize_bits(stream, lo, 32);
            serialize_bits(stream, hi, 32);

            if (Stream::IsReading)
            {
                value = (uint64_t(hi) << 32) | lo;
            }
            return true;
        }

        template <typename Stream>
        bool serializeBytes(Stream& stream, uint8_t* data, uint32_t bytes)
        {
            return stream.serializeBytes(data, bytes);
        }
    }
}

// use these to deserialize/serialize

// deserialization & serialization
#define serialize_int(stream, value, min, max)                                                           \
    do                                                                                                  \
    {                                                                                                   \
        ARENA_ASSERT(min < max, "min needs to be less than max");                                       \
        int32_t int32Value;                                                                             \
        if (Stream::IsWriting)                                                                          \
        {                                                                                               \
            ARENA_ASSERT(int64_t(value) >= int64_t(min), "Value needs to be more than min");            \
            ARENA_ASSERT(int64_t(value) <= int64_t(max), "Value needs to be less than max");            \
            int32Value = (int32_t)value;                                                                \
        }                                                                                               \
        if (!stream.serializeInteger(int32Value, min, max))                                             \
        {                                                                                               \
            return false;                                                                               \
        }                                                                                               \
        if (Stream::IsReading)                                                                          \
        {                                                                                               \
            value = int32Value;                                                                         \
            if (value < min || value > max)                                                             \
            {                                                                                           \
                return false;                                                                           \
            }                                                                                           \
        }                                                                                               \
    } while (0)

// deserialization & serialization
#define serialize_bool(stream, value)                                                                   \
    do                                                                                                  \
    {                                                                                                   \
        uint32_t boolvalue;                                                                             \
        if (Stream::IsWriting)                                                                          \
        {                                                                                               \
            boolvalue = value != 0;                                                                     \
        }                                                                                               \
        serialize_bits(stream, boolvalue, 1);                                                           \
        if (Stream::IsReading)                                                                          \
        {                                                                                               \
            boolvalue = boolvalue != 0;                                                                 \
        }                                                                                               \
    } while (0)

// deserialization & serialization
#define serialize_float(stream, value)                                                                  \
    do                                                                                                  \
    {                                                                                                   \
        if (!arena::detail::serializeFloat(stream, value))                                              \
        {                                                                                               \
            return false;                                                                               \
        }                                                                                               \
    } while (0)

// deserialization & serialization
#define serialize_uint32(stream, value)                                                                 \
    serialize_bits(stream, value, 32)

// deserialization & serialization
#define serialize_uint64(stream, value)                                                                 \
    do                                                                                                  \
    {                                                                                                   \
        if (!arena::detail::serializeUint64(stream, value))                                             \
        {                                                                                               \
            return false;                                                                               \
        }                                                                                               \
    } while (0)

#define serialize_bytes(stream, data, bytes)                                                            \
    do                                                                                                  \
    {                                                                                                   \
        if (!arena::detail::serializeBytes(stream, data, bytes)                                         \
        {                                                                                               \
            return false;                                                                               \
        }                                                                                               \
    } while (0)