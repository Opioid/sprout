#include "sphere_surrounding.hpp"
#include "base/math/vector.inl"

namespace scene { namespace surrounding {

Sphere::Sphere(std::shared_ptr<image::Image> image) : texture_(image) {}

math::float3 Sphere::sample(const math::Oray& ray) const {
	return math::float3(0.4, 0.f, 0.f);
}

}}
