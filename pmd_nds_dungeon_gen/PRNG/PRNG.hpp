#pragma once
#include <stdint.h>


class PRNG {
private:
    bool is_primary;
    uint32_t upper; //upper half of rng state
    uint32_t lower; //lower half
public:
    PRNG(const uint32_t, bool);
    PRNG(const PRNG&);
    uint32_t Rand16Bit();  //Note 32-bit
    uint16_t RandInt(const unsigned int);
    uint16_t Rand100();
    bool RandOutcome(const int);
    uint16_t RandRange(const int, const int);
};