#pragma once

#include "material_sample.hpp"
#include "base/math/vector.inl"
#include <cmath>

namespace scene { namespace material {

inline void Sample::Layer::set_basis(float3_p t, float3_p b, float3_p n) {
	this->t = t;
	this->b = b;
	this->n = n;
}

inline float Sample::Layer::clamped_n_dot(float3_p v) const {
	// return std::max(math::dot(n, v), 0.00001f);
	return math::clamp(math::dot(n, v), 0.00001f, 1.f);
}

inline float3_p Sample::Layer::shading_normal() const {
	return n;
}

inline float3 Sample::Layer::tangent_to_world(float3_p v) const {
	return float3(v.x * t.x + v.y * b.x + v.z * n.x,
				  v.x * t.y + v.y * b.y + v.z * n.y,
				  v.x * t.z + v.y * b.z + v.z * n.z);
}

inline float3_p Sample::geometric_normal() const {
	return geo_n_;
}

inline bool Sample::same_hemisphere(float3_p v) const {
	return math::dot(geo_n_, v) > 0.f;
}

inline void Sample::set_basis(float3_p geo_n, float3_p wo) {
	geo_n_ = geo_n;
	wo_    = wo;
}

inline float3 Sample::attenuation(float3_p color, float distance) {
	return float3(color.x > 0.f ? 1.f / (color.x * distance) : 0.f,
				  color.y > 0.f ? 1.f / (color.y * distance) : 0.f,
				  color.z > 0.f ? 1.f / (color.z * distance) : 0.f);
}

}}
