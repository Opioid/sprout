#pragma once

#include "scene/shape/shape_vertex.hpp"
#include "base/math/ray.hpp"

namespace scene { namespace shape { namespace triangle {

struct Triangle_MTE {
	Triangle_MTE(const shape::Vertex& a, const shape::Vertex& b, const shape::Vertex& c,
				 float bitangent_sign, uint32_t material_index);

	float3 ap, e1, e2;
	float3 an, bn, cn;
	float3 at, bt, ct;
	float2 auv, buv, cuv;
	float bitangent_sign;
	uint32_t material_index;

	bool intersect(math::Ray& ray, float2& uv) const;
	bool intersect_p(const math::Ray& ray) const;

	void interpolate(float2 uv, float3& p, float2& tc) const;
	void interpolate_data(float2 uv, float3& n, float3& t, float2& tc) const;
	float2 interpolate_uv(float2 uv) const;

	float3 normal() const;

	float area() const;
	float area(const float3& scale) const;
};

struct alignas(16) Intersection_triangle_MTE {
	Intersection_triangle_MTE() = default;
	Intersection_triangle_MTE(const shape::Vertex& a,
							  const shape::Vertex& b,
							  const shape::Vertex& c);

	float3 ap;
	float3 e1;
	float3 e2;

	bool intersect(math::Ray& ray, float2& uv) const;
	bool intersect_p(const math::Ray& ray) const;

	void interpolate(float2 uv, float3& p) const;

	float3 normal() const;

	float area() const;
	float area(const float3& scale) const;
};

struct alignas(16) Shading_triangle_MTE {
	Shading_triangle_MTE() = default;
	Shading_triangle_MTE(const shape::Vertex& a, const shape::Vertex& b, const shape::Vertex& c,
						 uint32_t material_index);

	float3 an;
	float3 bn;
	float3 cn;
	float3 at;
	float3 bt;
	float3 ct;

	float2 auv;
	float2 buv;
	float2 cuv;

	float bitangent_sign;
	uint32_t material_index;

	void interpolate_data(float2 uv, float3& n, float3& t, float2& tc) const;
	float2 interpolate_uv(float2 uv) const;
};

}}}
