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

	math::float3 tangent_to_world(math::float2 v) const{
		return math::float3(
			v.x * t.x + v.y * b.x,
			v.x * t.y + v.y * b.y,
			v.x * t.z + v.y * b.z);
	}

	void revert_direction() {
		n = -n;
		geo_n = -geo_n;
	}
};

}}
