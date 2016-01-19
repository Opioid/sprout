#pragma once

#include "base/math/vector.hpp"

namespace scene { namespace shape {

struct Differential {
	math::float3 p;			// posisition in world space
	math::float3 t, b, n;	// interpolated tangent frame in world space
	math::float3 geo_n;		// geometry normal in world space
	math::float2 uv;		// texture coordinates

	math::float3 tangent_to_world(const math::float3& v) const;

	math::float3 tangent_to_world(math::float2 v) const;

	bool same_hemisphere(const math::float3& v) const;

	void revert_direction();
};

}}
