#include "homogeneous.hpp"
#include "base/math/ray.inl"
#include "base/math/vector4.inl"
#include "base/math/bounding/aabb.inl"

namespace scene { namespace volume {

Homogeneous::Homogeneous(float3_p absorption, float3_p scattering, float anisotropy) :
	absorption_(absorption), scattering_(scattering), anisotropy_(anisotropy)
{}

float3 Homogeneous::optical_depth(const math::Oray& ray) const {
	return ray.length() * (absorption_ + scattering_);
}

float3 Homogeneous::scattering() const {
	return scattering_;
}

float phase_shlick(float3_p w, float3_p wp, float k) {
	float d = 1.f - (k * math::dot(w, wp));
	return 1.f / (4.f * math::Pi) * (1.f - k * k) / (d * d);
}

float Homogeneous::phase(float3_p w, float3_p wp) const {
	float g = anisotropy_;
	float k = 1.55f * g - 0.55f * g * g * g;
	return phase_shlick(w, wp, k);
}

}}
