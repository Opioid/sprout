#pragma once

#include "scene/shape/geometry/vertex.hpp"
#include "base/math/ray.hpp"

namespace scene { namespace shape { namespace triangle {

struct Triangle_MTE {
	Triangle_MTE(const shape::Vertex& a, const shape::Vertex& b, const shape::Vertex& c, uint32_t material_index);

	math::float3 ap, e1, e2;
	math::float3 an, bn, cn;
	math::float3 at, bt, ct;
	math::float2 auv, buv, cuv;
	float bitangent_sign;
	uint32_t material_index;

	bool intersect(math::Oray& ray, math::float2& uv) const;
	bool intersect_p(const math::Oray& ray) const;

	void interpolate(math::float2 uv, math::float3& p, math::float2& tc) const;
	void interpolate_data(math::float2 uv, math::float3& n, math::float3& t, math::float2& tc) const;
	math::float2 interpolate_uv(math::float2 uv) const;

	math::float3 normal() const;

	float area() const;
	float area(const math::float3& scale) const;
};

struct Intersection_triangle_MTE {
	Intersection_triangle_MTE();
	Intersection_triangle_MTE(const shape::Vertex& a, const shape::Vertex& b, const shape::Vertex& c);

	math::float3 ap; float pad0;
	math::float3 e1; float pad1;
	math::float3 e2; float pad2;
	math::float3 n; float pad3;

	bool intersect(math::Oray& ray, math::float2& uv) const;
	bool intersect_p(const math::Oray& ray) const;

	void interpolate(math::float2 uv, math::float3& p) const;

	math::float3 normal() const;

	float area() const;
	float area(const math::float3& scale) const;
};

struct Shading_triangle_MTE {
	Shading_triangle_MTE();
	Shading_triangle_MTE(const shape::Vertex& a, const shape::Vertex& b, const shape::Vertex& c, uint32_t material_index);

	math::float3 an; float pad0;
	math::float3 bn; float pad1;
	math::float3 cn; float pad2;
	math::float3 at; float pad3;
	math::float3 bt; float pad4;
	math::float3 ct; float pad5;

	math::float2 auv;
	math::float2 buv;
	math::float2 cuv;

	float bitangent_sign;
	uint32_t material_index;

	void interpolate_data(math::float2 uv, math::float3& n, math::float3& t, math::float2& tc) const;
	math::float2 interpolate_uv(math::float2 uv) const;
};

}}}
