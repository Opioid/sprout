#pragma once

#include "material_sample.hpp"
#include "base/math/vector.inl"
#include <cmath>

namespace scene { namespace material {

inline void Sample::Layer::set_basis(float3_p t, float3_p b, float3_p n, float sign) {
	this->t = t;
	this->b = b;
	this->n = sign * n;
}

inline float Sample::Layer::clamped_n_dot(float3_p v) const {
	return std::max(math::dot(n, v), 0.00001f);
}

inline float3 Sample::Layer::tangent_to_world(float3_p v) const {
	return float3(
		v.x * t.x + v.y * b.x + v.z * n.x,
		v.x * t.y + v.y * b.y + v.z * n.y,
		v.x * t.z + v.y * b.z + v.z * n.z);
}

inline float3_p Sample::geometric_normal() const {
	return geo_n_;
}

inline bool Sample::same_hemisphere(float3_p v) const {
	return math::dot(geo_n_, v) > 0.f;
}

inline float Sample::set_basis(float3_p geo_n, float3_p wo, bool two_sided) {
	wo_ = wo;

	if (two_sided && math::dot(geo_n, wo) < 0.f) {
		geo_n_ = -geo_n;
		return -1.f;
	} else {
		geo_n_ = geo_n;
		return 1.f;
	}
}

inline float3 Sample::attenuation(float3_p color, float distance) {
	return float3(
		color.x > 0.f ? 1.f / (color.x * distance) : 0.f,
		color.y > 0.f ? 1.f / (color.y * distance) : 0.f,
		color.z > 0.f ? 1.f / (color.z * distance) : 0.f);
}

}}
