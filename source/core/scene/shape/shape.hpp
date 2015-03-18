#pragma once

#include "base/math/bounding/aabb.hpp"

namespace scene {

struct Composed_transformation;

namespace shape {

struct Intersection;

class Shape {
public:

	const math::AABB& aabb() const;

	virtual bool intersect(const Composed_transformation& transformation, const math::Oray& ray, const math::float2& bounding,
						   Intersection& intersection, float& hit_t) const = 0;
	virtual bool intersect_p(const Composed_transformation& transformation, const math::Oray& ray, const math::float2& bounding) const = 0;

	virtual bool is_complex() const;
	virtual bool is_finite() const;

protected:

	math::AABB aabb_;
};

}}
