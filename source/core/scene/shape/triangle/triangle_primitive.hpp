#pragma once

#include <cstdint>

namespace scene { namespace shape { namespace triangle {

struct Index_triangle {
	Index_triangle() {}

	Index_triangle(uint32_t ai, uint32_t bi, uint32_t ci, uint32_t mi) :
		a(ai), b(bi), c(ci), material_index(mi) {}

	union {
		struct {
			uint32_t a, b, c;
		};

		uint32_t i[3];
	};

	uint32_t material_index;
};

}}}
