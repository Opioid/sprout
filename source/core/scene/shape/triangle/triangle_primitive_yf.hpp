#pragma once

#include "scene/shape/geometry/vertex.hpp"
#include "base/math/ray.hpp"

namespace scene { namespace shape { namespace triangle {

struct Triangle_YF {
	Vertex a, b, c;
	math::vec3 n; float d;
	math::vec3 n1; float d1;
	math::vec3 n2; float d2;
	uint32_t material_index;

	Triangle_YF(const Vertex& a, const Vertex& b, const Vertex& c, uint32_t material_index);

	bool intersect(math::Oray& ray, math::float2& uv) const;
	bool intersect_p(const math::Oray& ray) const;

	void interpolate(math::float2 uv, math::vec3& p, math::vec3& n, math::float2& tc) const;
	void interpolate_data(math::float2 uv, math::vec3& n, math::vec3& t, math::float2& tc) const;
	math::float2 interpolate_uv(math::float2 uv) const;

	math::vec3 normal() const;

	float area(const math::vec3& scale) const;
};

}}}
