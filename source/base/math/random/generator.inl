#pragma once

#include "generator.hpp"

namespace math { namespace random {

inline Generator::Generator() {}

inline Generator::Generator(uint32_t seed0, uint32_t seed1, uint32_t seed2, uint32_t seed3) {
	seed(seed0, seed1, seed2, seed3);
}

inline void Generator::seed(uint32_t seed0, uint32_t seed1, uint32_t seed2, uint32_t seed3) {
	z0 = seed0 | 128;
	z1 = seed1 | 128;
	z2 = seed2 | 128;
	z3 = seed3 | 128;
}

inline float Generator::random_float() {
	uint32_t bits = advance_lfsr113();

	return 2.3283064365387e-10f * static_cast<float>(bits);
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

}}
