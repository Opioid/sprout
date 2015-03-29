#include "sphere_surrounding.hpp"
#include "base/math/vector.inl"

namespace scene { namespace surrounding {

Sphere::Sphere(std::shared_ptr<image::texture::Texture> texture) {}

math::float3 Sphere::sample(const math::Oray& ray) const {
	return math::float3(0.4, 1.f, 0.2f);
}

}}
