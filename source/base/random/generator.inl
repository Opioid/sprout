#pragma once

#include "generator.hpp"

namespace rnd {
/*
inline Generator::Generator(uint32_t seed0, uint32_t seed1, uint32_t seed2, uint32_t seed3) :
	z0(seed0 | 128), z1(seed1 | 128), z2(seed2 | 128), z3(seed3 | 128) {}

inline float Generator::random_float() {
	const uint32_t bits = advance_lfsr113();

	return 2.3283064365386963e-10f * static_cast<float>(bits);
}

inline uint32_t Generator::random_uint() {
	return advance_lfsr113();
}

inline uint32_t Generator::advance_lfsr113() {
	uint32_t b;
	b  = ((z0 << 6) ^ z0) >> 13;
	z0 = ((z0 & uint32_t(4294967294)) << 18) ^ b;
	b  = ((z1 << 2) ^ z1) >> 27;
	z1 = ((z1 & uint32_t(4294967288)) << 2) ^ b;
	b  = ((z2 << 13) ^ z2) >> 21;
	z2 = ((z2 & uint32_t(4294967280)) << 7) ^ b;
	b  = ((z3 << 3) ^ z3) >> 12;
	z3 = ((z3 & uint32_t(4294967168)) << 13) ^ b;

	return z0 ^ z1 ^ z2 ^ z3;
}
*/

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
	const uint32_t bits = advance_pcg32();

	return 2.3283064365386963e-10f * static_cast<float>(bits);
}

inline uint32_t Generator::advance_pcg32() {
	uint64_t oldstate = state_;

	// Advance internal state
	state_ = oldstate * 6364136223846793005ULL + (inc_ | 1);

	// Calculate output function (XSH RR), uses old state for max ILP
	uint32_t xorshifted = static_cast<uint32_t>(((oldstate >> 18u) ^ oldstate) >> 27u);

	uint32_t rot = static_cast<uint32_t>(oldstate >> 59u);

	return (xorshifted >> rot) | (xorshifted << ((0xFFFFFFFF - rot) & 31));
}

}
