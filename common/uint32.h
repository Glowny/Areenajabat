#pragma once

#include <stdint.h>
#include <bx/platform.h> // macros
#include <bx/endian.h> // endianSwap
namespace arena
{
    // Compile time popcount
    template <uint32_t x> struct PopCount
    {
        enum {
            a = x - ((x >> 1) & 0x55555555),
            b = (((a >> 2) & 0x33333333) + (a & 0x33333333)),
            c = (((b >> 4) + b) & 0x0f0f0f0f),
            d = c + (c >> 8),
            e = d + (d >> 16),
            result = e & 0x0000003f
        };
    };

    // Compile time
    template <uint32_t x> struct Log2
    {
        enum {
            a = x | (x >> 1),
            b = a | (a >> 2),
            c = b | (b >> 4),
            d = c | (c >> 8),
            e = d | (d >> 16),
            f = e >> 1,
            result = PopCount<f>::result
        };
    };

    // Compile time 
    template <int64_t min, int64_t max> struct BitsRequired
    {
        static const uint32_t result = (min == max) ? 0 : (Log2<uint32_t(max - min)>::result + 1);
    };

    // runtime implementations

    inline uint32_t popcount(uint32_t x)
    {
#if BX_COMPILER_MSVC
        const uint32_t a = x - ((x >> 1) & 0x55555555);
        const uint32_t b = (((a >> 2) & 0x33333333) + (a & 0x33333333));
        const uint32_t c = (((b >> 4) + b) & 0x0f0f0f0f);
        const uint32_t d = c + (c >> 8);
        const uint32_t e = d + (d >> 16);
        const uint32_t result = e & 0x0000003f;
        return result;
#else
        return __builtin_popcount(x);
#endif
    }

#if BX_COMPILER_MSVC
    inline uint32_t log2(uint32_t x)
    {
        const uint32_t a = x | (x >> 1);
        const uint32_t b = a | (a >> 2);
        const uint32_t c = b | (b >> 4);
        const uint32_t d = c | (c >> 8);
        const uint32_t e = d | (d >> 16);
        const uint32_t f = e >> 1;
        return popcount(f);
    }

    inline int bitsRequired(uint32_t min, uint32_t max)
    {
        return (min == max) ? 0 : log2(max - min) + 1;
    }
#else
    inline int bitsRequired(uint32_t min, uint32_t max)
    {
        return 32 - __builtin_clz(max - min);
    }
#endif

    // Network is in little endian, convert value to little endian if host is big endian
    template <typename T>
    inline T hostToNetwork(T value)
    {
#if BX_CPU_ENDIAN_BIG
        return bx::endianSwap(value)
#else
        return value;
#endif
    }


    // Network is in little endian, convert it to big endian if host is big endian
    template <typename T>
    inline T networkToHost(T value)
    {
#if BX_CPU_ENDIAN_BIG
        return bx::endianSwap(value)
#else
        return value;
#endif
    }
}