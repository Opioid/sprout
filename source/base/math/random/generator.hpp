#pragma once

#include <cstdint>

namespace math { namespace random {

class Generator {
public:

	Generator();
	Generator(uint32_t seed0, uint32_t seed1, uint32_t seed2, uint32_t seed3);

	void seed(uint32_t seed0, uint32_t seed1, uint32_t seed2, uint32_t seed3);

	float random_float();
	uint32_t random_uint();

private:

	uint32_t advance_lfsr113();

	uint32_t z0, z1, z2, z;
};

}}
