#pragma once

#include "base/math/vector.hpp"

namespace scene { namespace shape {

struct Differential {
	math::vec3 p;			// posisition in world space
	math::vec3 t, b, n;	// interpolated tangent frame in world space
	math::vec3 geo_n;		// geometry normal in world space
	math::float2 uv;		// texture coordinates

	math::vec3 tangent_to_world(const math::vec3& v) const;

	math::vec3 tangent_to_world(math::float2 v) const;

	bool same_hemisphere(const math::vec3& v) const;

	void revert_direction();
};

}}
