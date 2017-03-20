#pragma once

#include <cstdint>

namespace math {

/****************************************************************************
 *
 * Aligned 8D unsigned short vector
 *
 ****************************************************************************/

struct alignas(16) Vector8us_a {
	uint16_t v[8];

	Vector8us_a() = default;

	uint16_t operator[](uint32_t i) const {
		return v[i];
	}

	uint16_t& operator[](uint32_t i) {
		return v[i];
	}
};

}

using ushort8 = Vector8us_a;

