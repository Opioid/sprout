#pragma once

#include "base/math/vector.hpp"

namespace scene { namespace shape {

struct Differential {
	math::float3 p;			// posisition in world space
	math::float3 t, b, n;	// interpolated tangent frame in world space
	math::float3 geo_n;		// geometry normal in world space
	math::float2 uv;		// texture coordinates

	math::float3 tangent_to_world(const math::float3& v) const{
		return math::float3(
			v.x * t.x + v.y * b.x + v.z * n.x,
			v.x * t.y + v.y * b.y + v.z * n.y,
			v.x * t.z + v.y * b.z + v.z * n.z);
	}

	math::float3 tangent_to_world_zyx(math::float2 v) const{
		return math::float3(
			v.x * n.x + v.y * b.x + t.x,
			v.x * n.y + v.y * b.y + t.y,
			v.x * n.z + v.y * b.z + t.z);
	}
};

}}
