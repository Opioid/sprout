#ifndef SU_BASE_RANDOM_GENERATOR_INL
#define SU_BASE_RANDOM_GENERATOR_INL

#include "generator.hpp"

namespace rnd {

inline Generator::Generator() noexcept = default;

inline Generator::Generator(uint64_t state, uint64_t sequence) noexcept {
    start(state, sequence);
}

inline void Generator::start(uint64_t state, uint64_t sequence) noexcept {
    state_.state = 0;
    state_.inc   = (sequence << 1u) | 1u;

    random_uint();
    state_.state += state;
    random_uint();
}

inline Generator::State const& Generator::state() const noexcept {
    return state_;
}

inline void Generator::set_state(State const& state) noexcept {
    state_ = state;
}

inline uint32_t Generator::random_uint() noexcept {
    return advance_pcg32();
}

inline float Generator::random_float() noexcept {
    uint32_t const bits = advance_pcg32();

    return 2.3283064365386963e-10f * static_cast<float>(bits);
}

inline uint32_t Generator::advance_pcg32() noexcept {
    uint64_t const oldstate = state_.state;

    // Advance internal state
    state_.state = oldstate * 6364136223846793005ull + (state_.inc | 1);

    // Calculate output function (XSH RR), uses old state for max ILP
    uint32_t const xorshifted = static_cast<uint32_t>(((oldstate >> 18ull) ^ oldstate) >> 27ull);

    uint32_t const rot = static_cast<uint32_t>(oldstate >> 59ull);

    return (xorshifted >> rot) | (xorshifted << ((0xFFFFFFFFu - rot) & 31u));
}

}  // namespace rnd

#endif
