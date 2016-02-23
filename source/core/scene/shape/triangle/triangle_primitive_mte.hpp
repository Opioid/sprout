#pragma once

#include "scene/shape/geometry/vertex.hpp"
#include "base/math/ray.hpp"

namespace scene { namespace shape { namespace triangle {

struct Triangle_MTE {
	Triangle_MTE(const shape::Vertex& a, const shape::Vertex& b, const shape::Vertex& c, uint32_t material_index);

	math::vec3 ap, e1, e2;
	math::vec3 an, bn, cn;
	math::vec3 at, bt, ct;
	math::float2 auv, buv, cuv;
	float bitangent_sign;
	uint32_t material_index;

	bool intersect(math::Oray& ray, math::float2& uv) const;
	bool intersect_p(const math::Oray& ray) const;

	void interpolate(math::float2 uv, math::vec3& p, math::float2& tc) const;
	void interpolate_data(math::float2 uv, math::vec3& n, math::vec3& t, math::float2& tc) const;
	math::float2 interpolate_uv(math::float2 uv) const;

	math::vec3 normal() const;

	float area() const;
	float area(const math::vec3& scale) const;
};

struct alignas(16) Intersection_triangle_MTE {
	Intersection_triangle_MTE();
	Intersection_triangle_MTE(const shape::Vertex& a, const shape::Vertex& b, const shape::Vertex& c);

	math::vec3 ap;
	math::vec3 e1;
	math::vec3 e2;
	math::vec3 n;

	bool intersect(math::Oray& ray, math::float2& uv) const;
	bool intersect_p(const math::Oray& ray) const;

	void interpolate(math::float2 uv, math::vec3& p) const;

	math::vec3 normal() const;

	float area() const;
	float area(const math::vec3& scale) const;
};

struct alignas(16) Shading_triangle_MTE {
	Shading_triangle_MTE();
	Shading_triangle_MTE(const shape::Vertex& a, const shape::Vertex& b, const shape::Vertex& c, uint32_t material_index);

	math::vec3 an;
	math::vec3 bn;
	math::vec3 cn;
	math::vec3 at;
	math::vec3 bt;
	math::vec3 ct;

	math::float2 auv;
	math::float2 buv;
	math::float2 cuv;

	float bitangent_sign;
	uint32_t material_index;

	void interpolate_data(math::float2 uv, math::vec3& n, math::vec3& t, math::float2& tc) const;
	math::float2 interpolate_uv(math::float2 uv) const;
};

}}}
