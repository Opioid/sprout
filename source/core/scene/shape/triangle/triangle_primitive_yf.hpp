#pragma once

#include "scene/shape/geometry/vertex.hpp"
#include "base/math/ray.hpp"

namespace scene { namespace shape { namespace triangle {

struct Triangle_YF {
	Vertex a, b, c;
	float3 n; float d;
	float3 n1; float d1;
	float3 n2; float d2;
	uint32_t material_index;

	Triangle_YF(const Vertex& a, const Vertex& b, const Vertex& c, uint32_t material_index);

	bool intersect(math::Ray& ray, float2& uv) const;
	bool intersect_p(const math::Ray& ray) const;

	void interpolate(float2 uv, float3& p, float3& n, float2& tc) const;
	void interpolate_data(float2 uv, float3& n,
						  float3& t, float2& tc) const;
	float2 interpolate_uv(float2 uv) const;

	float3 normal() const;

	float area(const float3& scale) const;
};

}}}
