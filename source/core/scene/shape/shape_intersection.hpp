#ifndef SU_CORE_SCENE_SHAPE_INTERSECTION_HPP
#define SU_CORE_SCENE_SHAPE_INTERSECTION_HPP

#include "base/math/vector2.hpp"
#include "base/math/vector3.hpp"

namespace scene::shape {

struct Intersection {
	float3 p;		// position in world space
	float3 geo_n;	// geometry normal in world space
	float3 t, b, n;	// interpolated tangent frame in world space
	float2 uv;		// texture coordinates

	float epsilon;
	uint32_t part;
	bool subsurface;
};

}

#endif
