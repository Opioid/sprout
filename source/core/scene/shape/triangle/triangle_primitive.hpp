#ifndef SU_CORE_SCENE_SHAPE_TRIANGLE_PRIMITIVE_HPP
#define SU_CORE_SCENE_SHAPE_TRIANGLE_PRIMITIVE_HPP

#include <cstdint>

namespace scene::shape::triangle {

struct Index_triangle {
	Index_triangle() = default;

	Index_triangle(uint32_t a, uint32_t b, uint32_t c, uint32_t mi) :
		i{a, b, c}, material_index(mi) {}

	uint32_t i[3];

	uint32_t material_index;
};

}

#endif
