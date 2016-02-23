#pragma once

#include "scene/shape/geometry/vertex.hpp"
#include "base/math/ray.hpp"

namespace scene { namespace shape { namespace triangle {

struct Triangle_MT {
	Triangle_MT(const shape::Vertex& a, const shape::Vertex& b, const shape::Vertex& c, uint32_t material_index);

	struct Vertex {
		Vertex(const shape::Vertex& v);

		math::vec3 p, n, t;
		math::float2 uv;
	};

	Vertex a, b, c;
	float bitangent_sign;
	uint32_t material_index;

	bool intersect(math::Oray& ray, math::float2& uv) const;
	bool intersect_p(const math::Oray& ray) const;

	void interpolate(math::float2 uv, math::vec3& p, math::vec3& n, math::float2& tc) const;
	void interpolate(math::float2 uv, math::vec3& p, math::float2& tc) const;
	void interpolate(math::float2 uv, math::vec3& p) const;
	void interpolate_data(math::float2 uv, math::vec3& n, math::vec3& t, math::float2& tc) const;
	math::float2 interpolate_uv(math::float2 uv) const;

	math::vec3 normal() const;

	float area() const;
	float area(const math::vec3& scale) const;
};

}}}
