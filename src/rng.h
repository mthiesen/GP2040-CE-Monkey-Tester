#pragma once

#include "hardware/structs/rosc.h"

#include <cstdint>

class KissRNG
{
public:
    KissRNG() :
        x(generateSeedValue()),
        y(362436000),
        z(521288629),
        c(7654321)
    {}

    uint32_t next()
    {
        x = 69069 * x + 12345;
        y ^= y << 13;
        y ^= y >> 17;
        y ^= y << 5;
        const uint64_t t = static_cast<uint64_t>(698769069) * z + c;
        c = t >> 32;
        z = static_cast<uint32_t>(t);
        return x + y + z;
    }

private:
    static uint32_t generateSeedValue()
    {
        uint32_t seedValue = 0;
        for (int i = 0; i < 32; ++i)
        {
            seedValue = (seedValue << 1) | rosc_hw->randombit;
        }
        return seedValue;
    }

    uint32_t x;
    uint32_t y;
    uint32_t z;
    uint32_t c;
};
