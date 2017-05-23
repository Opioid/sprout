#pragma once

#include <cstdint>

namespace rnd {
/*
class Generator {

public:

	Generator() = default;

	// The initial seeds seed0, seed1, seed2, seed3
	// must be larger than 1, 7, 15, and 127 respectively.
	Generator(uint32_t seed0, uint32_t seed1, uint32_t seed2, uint32_t seed3);

//	void seed(uint32_t seed0, uint32_t seed1, uint32_t seed2, uint32_t seed3);

	float random_float();
	uint32_t random_uint();

private:

	uint32_t advance_lfsr113();

	uint32_t z0, z1, z2, z3;
};
*/

// http://www.pcg-random.org/

class Generator {

public:

	Generator() = default;

	Generator(uint64_t state, uint64_t sequence);

	uint32_t random_uint();
	float random_float();

private:

	uint32_t advance_pcg32();

	uint64_t state_;
	uint64_t inc_;
};

}
