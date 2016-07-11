#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <enet/enet.h>
#include <bx/platform.h>

namespace arena
{
    uint64_t genSalt()
    {
        return  ((uint64_t(rand()) << 0) & 0x000000000000FFFFull)  |
                ((uint64_t(rand()) << 16) & 0x00000000FFFF0000ull) |
                ((uint64_t(rand()) << 32) & 0x0000FFFF00000000ull) |
                ((uint64_t(rand()) << 48) & 0xFFFF000000000000ull);
    }

    uint64_t murmur_hash_64(const void * key, uint32_t length, uint64_t seed)
    {
        const uint64_t m = 0xc6a4a7935bd1e995ULL;
        const uint32_t r = 47;

        uint64_t h = seed ^ (length * m);

        const uint64_t * data = (const uint64_t*)key;
        const uint64_t * end = data + length / 8;

        while (data != end)
        {
#if BX_CPU_ENDIAN_LITTLE
            uint64_t k = *data++;
#else 
            uint64_t k = *data++;
            uint8_t * p = (uint8_t*)&k;
            uint8_t c;
            c = p[0]; p[0] = p[7]; p[7] = c;
            c = p[1]; p[1] = p[6]; p[6] = c;
            c = p[2]; p[2] = p[5]; p[5] = c;
            c = p[3]; p[3] = p[4]; p[4] = c;
#endif

            k *= m;
            k ^= k >> r;
            k *= m;

            h ^= k;
            h *= m;
        }

        const uint8_t * data2 = (const uint8_t*)data;

        switch (length & 7)
        {
        case 7: h ^= uint64_t(data2[6]) << 48;
        case 6: h ^= uint64_t(data2[5]) << 40;
        case 5: h ^= uint64_t(data2[4]) << 32;
        case 4: h ^= uint64_t(data2[3]) << 24;
        case 3: h ^= uint64_t(data2[2]) << 16;
        case 2: h ^= uint64_t(data2[1]) << 8;
        case 1: h ^= uint64_t(data2[0]);
            h *= m;
        };

        h ^= h >> r;
        h *= m;
        h ^= h >> r;

        return h;
    }

    uint64_t calculateChallengeHash(ENetPeer* peer, uint64_t clientSalt, uint64_t serverSeed)
    {
        char addressString[256];
        enet_address_get_host_ip(&peer->address, addressString, sizeof(addressString));
        const uint32_t len = (uint32_t)strlen(addressString);
        return murmur_hash_64(
            &serverSeed,
            8u,
            murmur_hash_64(&clientSalt, 8, murmur_hash_64(addressString, len, 0))
        );
    }
}