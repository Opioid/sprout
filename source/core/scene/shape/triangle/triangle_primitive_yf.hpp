#pragma once

#include "scene/shape/geometry/vertex.hpp"
#include "base/math/ray.hpp"

namespace scene { namespace shape { namespace triangle {

struct Triangle_YF {
	Vertex a, b, c;
	math::float3 n; float d;
	math::float3 n1; float d1;
	math::float3 n2; float d2;
	uint32_t material_index;

	Triangle_YF(const Vertex& a, const Vertex& b, const Vertex& c, uint32_t material_index);

	bool intersect(math::Oray& ray, math::float2& uv) const;
	bool intersect_p(const math::Oray& ray) const;

	void interpolate(math::float2 uv, math::float3& p, math::float3& n, math::float2& tc) const;
	void interpolate_data(math::float2 uv, math::float3& n,
						  math::float3& t, math::float2& tc) const;
	math::float2 interpolate_uv(math::float2 uv) const;

	math::float3 normal() const;

	float area(const math::float3& scale) const;
};

}}}
