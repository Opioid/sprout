#ifndef SU_BASE_RANDOM_GENERATOR_INL
#define SU_BASE_RANDOM_GENERATOR_INL

#include "generator.hpp"

namespace rnd {

inline Generator::Generator(uint64_t state, uint64_t sequence) :
	state_(0), inc_((sequence << 1u) | 1u) {
	random_uint();
	state_ += state;
	random_uint();
}

inline uint32_t Generator::random_uint() {
	return advance_pcg32();
}

inline float Generator::random_float() {
	uint32_t const bits = advance_pcg32();

	return 2.3283064365386963e-10f * static_cast<float>(bits);
}

inline uint32_t Generator::advance_pcg32() {
	const uint64_t oldstate = state_;

	// Advance internal state
	state_ = oldstate * 6364136223846793005ULL + (inc_ | 1);

	// Calculate output function (XSH RR), uses old state for max ILP
	uint32_t const xorshifted = static_cast<uint32_t>(((oldstate >> 18u) ^ oldstate) >> 27u);

	uint32_t const rot = static_cast<uint32_t>(oldstate >> 59u);

	return (xorshifted >> rot) | (xorshifted << ((0xFFFFFFFF - rot) & 31));
}

}

#endif
