#pragma once

#include <cstdint>

namespace scene { namespace shape { namespace triangle {

struct Index_triangle {
	union {
		struct {
			uint32_t a, b, c;
		};

		uint32_t i[3];
	};

	uint32_t material_index;
};

}}}
