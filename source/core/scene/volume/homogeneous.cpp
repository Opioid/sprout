#include "homogeneous.hpp"
#include "base/math/ray.inl"
#include "base/math/vector4.inl"
#include "base/math/bounding/aabb.inl"

namespace scene { namespace volume {

Homogeneous::Homogeneous(float3_p absorption, float3_p scattering) :
	absorption_(absorption), scattering_(scattering)
{}

float3 Homogeneous::optical_depth(const math::Oray& ray) const {
	return ray.length() * (absorption_ + scattering_);
}

float3 Homogeneous::scattering() const {
	return scattering_;
}

float phase_shlick(float3_p w, float3_p wp, float k) {
	float kct = k * math::dot(w, wp);
	return 1.f / (4.f * math::Pi) * (1.f - k * k) / ((1.f - kct) * (1.f - kct));
}

float Homogeneous::phase(float3_p w, float3_p wp) const {
//	float g = 0.f;
//	float k = 1.55f * g - 0.55f * g * g * g;
	return phase_shlick(w, wp, 0.f);
}

}}
