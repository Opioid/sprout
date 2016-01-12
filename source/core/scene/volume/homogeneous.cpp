#include "homogeneous.hpp"
#include "base/math/ray.inl"

namespace scene { namespace volume {

Homogeneous::Homogeneous(const math::float3& absorption) : absorption_(absorption), scattering_(0.f, 0.f, 0.f)
{}

math::float3 Homogeneous::optical_depth(const math::Oray& ray) const {
	// At the moment we treat all volumes as basically infinite.
	// But this gives problems with sun/sky light that now has to penetrate basically infinite layers of absorption.
	// Therefore the infinite volume is bounded on the y axis.
	float hit_t = (atmosphere_y_ - ray.origin.y) * ray.reciprocal_direction.y;

	if (hit_t < 0.f || hit_t > ray.max_t) {
		hit_t = ray.max_t;
	}

	float d = math::distance(ray.point(ray.min_t), ray.point(hit_t));

	return d * (absorption_ + scattering_);
}

}}
