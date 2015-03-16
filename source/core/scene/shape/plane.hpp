#pragma once

#include "shape.hpp"

namespace scene { namespace shape {

class Plane : public Shape {
public:

	virtual bool intersect(const Composed_transformation& transformation, const math::Oray& ray, const math::float2& bounding,
						   Intersection& intersection, float& hit_t) const;
	virtual bool intersect_p(const Composed_transformation& transformation, const math::Oray& ray, const math::float2& bounding) const;

	virtual bool is_finite() const;
};

}}
