#include "homogeneous.hpp"
#include "base/math/ray.inl"
#include "base/math/vector4.inl"
#include "base/math/bounding/aabb.inl"

namespace scene { namespace volume {

float3 Homogeneous::optical_depth(const math::Oray& ray) const {
	return ray.length() * (absorption_ + scattering_);
}

float3 Homogeneous::scattering(float3_p /*p*/) const {
	return scattering_;
}

float Homogeneous::phase(float3_p w, float3_p wp) const {
	float g = anisotropy_;
	float k = 1.55f * g - 0.55f * g * g * g;
	return phase_schlick(w, wp, k);
}

void Homogeneous::set_parameter(const std::string& /*name*/, const json::Value& /*value*/) {}

}}
