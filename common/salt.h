#pragma once

#include <stdint.h>
#include <stdlib.h>

namespace arena
{
    uint64_t genSalt()
    {
        return  ((uint64_t(rand()) << 0) & 0x000000000000FFFFull)  |
                ((uint64_t(rand()) << 16) & 0x00000000FFFF0000ull) |
                ((uint64_t(rand()) << 32) & 0x0000FFFF00000000ull) |
                ((uint64_t(rand()) << 48) & 0xFFFF000000000000ull);
    }
}