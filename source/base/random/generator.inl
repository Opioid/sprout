#ifndef SU_BASE_RANDOM_GENERATOR_INL
#define SU_BASE_RANDOM_GENERATOR_INL

#include "generator.hpp"

#include <bit>

namespace rnd {

inline Generator::Generator() = default;

inline Generator::Generator(uint64_t state, uint64_t sequence) {
    start(state, sequence);
}

inline void Generator::start(uint64_t state, uint64_t sequence) {
    state_ = 0;
    inc_   = (sequence << 1u) | 1u;

    random_uint();
    state_ += state;
    random_uint();
}

inline uint32_t Generator::random_uint() {
    return advance_pcg32();
}

inline float Generator::random_float() {
    uint32_t bits = advance_pcg32();

    bits &= 0x007FFFFFu;
    bits |= 0x3F800000u;

    return std::bit_cast<float>(bits) - 1.f;
}

inline uint32_t Generator::advance_pcg32() {
    uint64_t const old = state_;

    // Advance internal state
    state_ = old * 6364136223846793005ull + (inc_ | 1);

    // Calculate output function (XSH RR), uses old state for max ILP
    uint32_t const xrs = uint32_t(((old >> 18ull) ^ old) >> 27ull);

    uint32_t const rot = uint32_t(old >> 59ull);

    return (xrs >> rot) | (xrs << ((0u - rot) & 31u));
}

}  // namespace rnd

#endif
