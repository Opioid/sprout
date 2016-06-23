#pragma once

#include "base/math/vector.hpp"

namespace scene { namespace shape {

struct Hitpoint {
	float3 p;			// posisition in world space
	float3 t, b, n;	// interpolated tangent frame in world space
	float3 geo_n;		// geometry normal in world space
	float2 uv;		// texture coordinates

	float3 tangent_to_world(float3_p v) const;

	float3 tangent_to_world(float2 v) const;

	bool same_hemisphere(float3_p v) const;

	void revert_direction();
};

}}
