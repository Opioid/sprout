#include "homogeneous.hpp"
#include "base/math/ray.inl"
#include "base/math/bounding/aabb.inl"

namespace scene { namespace volume {

Homogeneous::Homogeneous(const math::float3& absorption, const math::float3& scattering) :
	absorption_(absorption), scattering_(scattering)
{}

math::float3 Homogeneous::optical_depth(const math::Oray& ray) const {
	// At the moment we treat all volumes as basically infinite.
	// But this gives problems with sun/sky light that now has to penetrate basically infinite layers of absorption.
	// Therefore the infinite volume is bounded on the y axis.
	float atmosphere_y = scene_bb_.max().y;
	float hit_t = (atmosphere_y - ray.origin.y) * ray.reciprocal_direction.y;

	if (hit_t < 0.f || hit_t > ray.max_t) {
		hit_t = ray.max_t;
	}

	float d = math::distance(ray.point(ray.min_t), ray.point(hit_t));

	return d * (absorption_ + scattering_);
}

math::float3 Homogeneous::scattering() const {
	return scattering_;
}

float phase_shlick(const math::float3& w, const math::float3& wp, float g) {
	float k = 1.55f * g - 0.55f * g * g * g;
	float kct = k * math::dot(w, wp);
	return 1.f / (4.f * math::Pi) * (1.f - k * k) / ((1.f - kct) * (1.f - kct));
}

float Homogeneous::phase(const math::float3& w, const math::float3& wp) const {
	return phase_shlick(w, wp, 0.f);
}

}}
