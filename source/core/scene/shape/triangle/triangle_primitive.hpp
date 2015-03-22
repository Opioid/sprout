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
};

}}}
