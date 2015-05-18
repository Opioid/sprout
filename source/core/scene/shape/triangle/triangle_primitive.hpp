#pragma once

#include "scene/shape/geometry/vertex.hpp"
#include "base/math/ray.hpp"

namespace scene { namespace shape { namespace triangle {

struct Index_triangle {
	uint32_t a, b, c;
	uint32_t material_index;
};

struct Coordinates;

struct Triangle {
	Vertex a, b, c;
	uint32_t material_index;

	bool intersect(const math::Oray& ray, Coordinates& coordinates) const;
	bool intersect_p(const math::Oray& ray) const;

	void interpolate(math::float2 uv, math::float3& p, math::float3& n, math::float2& tc) const;
	void interpolate_data(math::float2 uv, math::float3& n, math::float3& t, math::float2& tc) const;
	math::float2 interpolate_uv(math::float2 uv) const;

	math::float3 normal() const;

	float area(const math::float3& scale) const;
};

}}}
