#include "density.hpp"
#include "base/math/ray.inl"
#include "base/math/vector4.inl"
#include "base/math/bounding/aabb.inl"

namespace scene { namespace volume {

float3 Density::optical_depth(const math::Oray& ray) const {
	return ray.length() * (absorption_ + scattering_);
}

float3 Density::scattering(float3_p p) const {
	return density(p) * scattering_;
}

float Density::phase(float3_p w, float3_p wp) const {
	float g = anisotropy_;
	float k = 1.55f * g - 0.55f * g * g * g;
	return phase_schlick(w, wp, k);
}

}}
