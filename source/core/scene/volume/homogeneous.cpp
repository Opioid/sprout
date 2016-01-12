#include "homogeneous.hpp"
#include "base/math/ray.inl"

namespace scene { namespace volume {

Homogeneous::Homogeneous(const math::float3& absorption) : absorption_(absorption), scattering_(0.f, 0.f, 0.f)
{}

math::float3 Homogeneous::optical_depth(const math::Oray& ray) const {
	return ray.length() * (absorption_ + scattering_);
}

}}
