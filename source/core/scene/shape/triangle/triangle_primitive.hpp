#pragma once

#include "scene/shape/geometry/vertex.hpp"
#include "base/math/ray.hpp"

namespace scene { namespace shape { namespace triangle {

struct Index_triangle {
	uint32_t a, b, c;
	uint32_t material_index;
};

struct Triangle {
	Vertex a, b, c;
	uint32_t material_index;

	bool intersect(math::Oray& ray, math::float2& uv) const;
	bool intersect_p(const math::Oray& ray) const;

	void interpolate(math::float2 uv, math::float3& p, math::float3& n, math::float2& tc) const;
	void interpolate_data(math::float2 uv, math::float3& n, math::float3& t, math::float2& tc) const;
	math::float2 interpolate_uv(math::float2 uv) const;

	math::float3 normal() const;

	float area(const math::float3& scale) const;
};

struct Position_triangle {
	math::float3 a, b, c;

	bool intersect(math::Oray& ray, math::float2& uv) const;
	bool intersect_p(const math::Oray& ray) const;

	math::float3 normal() const;

	float area(const math::float3& scale) const;
};

struct Data_triangle {
	Data_vertex a, b, c;
	uint32_t material_index;

	void interpolate_data(math::float2 uv, math::float3& n, math::float3& t, math::float2& tc) const;
	math::float2 interpolate_uv(math::float2 uv) const;
};

bool intersect(const math::float3& a, const math::float3& b, const math::float3& c, math::Oray& ray, math::float2& uv);
bool intersect_p(const math::float3& a, const math::float3& b, const math::float3& c, const math::Oray& ray);

void interpolate(const Vertex& a, const Vertex& b, const Vertex& c, math::float2 uv, math::float3& p, math::float3& n, math::float2& tc);
void interpolate_data(const Vertex& a, const Vertex& b, const Vertex& c, math::float2 uv, math::float3& n, math::float3& t, math::float2& tc);
math::float2 interpolate_uv(math::float2 a, math::float2 b, math::float2 c, math::float2 uv);

math::float3 normal(const math::float3& a, const math::float3& b, const math::float3& c);

float area(const math::float3& a, const math::float3& b, const math::float3& c, const math::float3& scale);

}}}
