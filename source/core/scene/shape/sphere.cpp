#include "sphere.hpp"

namespace scene { namespace shape {

bool Sphere::intersect(const Composed_transformation& transformation, const math::Oray& ray, const math::float2& bounding,
					   Intersection& intersection, float& hit_t) const {
	return false;
}

bool Sphere::intersect_p(const Composed_transformation& transformation, const math::Oray& ray, const math::float2& bounding) const {
	return false;
}

}}
