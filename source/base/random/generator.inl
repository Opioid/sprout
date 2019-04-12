#ifndef SU_BASE_RANDOM_GENERATOR_INL
#define SU_BASE_RANDOM_GENERATOR_INL

#include <cstring>
#include "generator.hpp"

namespace rnd {

inline Generator::Generator() noexcept = default;

inline Generator::Generator(uint64_t state, uint64_t sequence) noexcept {
    start(state, sequence);
}

inline void Generator::start(uint64_t state, uint64_t sequence) noexcept {
    state_ = 0;
    inc_   = (sequence << 1u) | 1u;

    random_uint();
    state_ += state;
    random_uint();
}

inline uint32_t Generator::random_uint() noexcept {
    return advance_pcg32();
}

static inline float uint_as_float(uint32_t x) noexcept {
    float f;
    std::memcpy(&f, &x, sizeof(float));
    return f;
}

inline float Generator::random_float() noexcept {
    uint32_t bits = advance_pcg32();

    bits &= 0x007FFFFFu;
    bits |= 0x3F800000u;

    return uint_as_float(bits) - 1.f;

    //   return 2.3283064365386963e-10f * static_cast<float>(bits);
}

inline uint32_t Generator::advance_pcg32() noexcept {
    uint64_t const old = state_;

    // Advance internal state
    state_ = old * 6364136223846793005ull + (inc_ | 1);

    // Calculate output function (XSH RR), uses old state for max ILP
    uint32_t const xrs = static_cast<uint32_t>(((old >> 18ull) ^ old) >> 27ull);

    uint32_t const rot = static_cast<uint32_t>(old >> 59ull);

    return (xrs >> rot) | (xrs << ((0u - rot) & 31u));
}

}  // namespace rnd

#endif
