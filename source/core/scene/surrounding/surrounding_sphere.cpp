#include "surrounding_sphere.hpp"
#include "base/math/vector.inl"

namespace scene { namespace surrounding {

Sphere::Sphere(std::shared_ptr<image::Image> image) : texture_(image) {}

math::float3 Sphere::sample(const math::Oray& ray) const {
	return sampler_nearest_.sample3(texture_, ray.direction);
}

}}
