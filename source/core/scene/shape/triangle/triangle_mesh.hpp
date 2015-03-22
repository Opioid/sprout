#pragma once

#include "scene/shape/shape.hpp"
#include "bvh/triangle_bvh_tree.hpp"

namespace scene { namespace shape { namespace triangle {

class Mesh : public Shape {
public:

	virtual bool intersect(const Composed_transformation& transformation, const math::Oray& ray, const math::float2& bounds,
						   shape::Intersection& intersection, float& hit_t) const;
	virtual bool intersect_p(const Composed_transformation& transformation, const math::Oray& ray, const math::float2& bounds) const;

	virtual bool is_complex() const;

private:

	bvh::Tree tree_;

	friend class Provider;
};

}}}

