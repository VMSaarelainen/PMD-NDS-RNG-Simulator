#include <iostream>
#include <stdint.h>
#include <optional>
#include "PRNG.hpp"


PRNG::PRNG(const uint32_t seed, bool is_primary) {
    this->is_primary = is_primary;
    upper = seed >> 16;
    lower = seed % UINT16_MAX;
}

PRNG::PRNG(const PRNG& other) {
    is_primary = other.is_primary;
    upper = other.upper;
    lower = other.lower;
}

/**
 * Random number in the range [0, uint16 max], but stored in a uint32 to reduce casting in the other functions
*/
uint32_t PRNG::Rand16Bit() {
    if (is_primary) {
        upper += 1;
        lower = lower * 0x5D588B65 +1;
        return (lower & 0xFFFF0000) >> 16;
    }
    else {
        std::cout << "WARN entered else in Rand16Bit\n";
        upper += 0x269EC3;  //todo look into this
        return upper;
    }
}

/**
 * Random number in the range [0, n-1]
*/
uint16_t PRNG::RandInt(const unsigned int n) {
    return (Rand16Bit() * n) >> 16;
}

/**
 * Randon number in the range [0, 100]
*/
uint16_t PRNG::Rand100() {
    return RandInt(101);
}

bool PRNG::RandOutcome(const int percentage) {
    auto x = PRNG::RandInt(100);
    return x < percentage ? false : true;
}

uint16_t PRNG::RandRange(const int x, const int y) {
    if (x == y) {
        return x;
    }
    if (x < y) {
        return x + ((Rand16Bit() * (y - x)) >> 0x10);
    } else {
        return y + ((Rand16Bit() * (x - y)) >> 0x10);
    }
}