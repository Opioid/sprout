#ifndef SU_CORE_SCENE_SHAPE_TRIANGLE_INTERSECTION_HPP
#define SU_CORE_SCENE_SHAPE_TRIANGLE_INTERSECTION_HPP

#include "base/simd/simd.hpp"
#include <cstdint>
//#include "base/math/vector3.hpp"

namespace scene::shape::triangle {

struct Intersection {
	Vector u;
	Vector v;

//	float2 uv;
	uint32_t index;
};

}

#endif
