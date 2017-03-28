#pragma once

#include "material_sample.hpp"
#include "base/math/vector3.inl"
#include <cmath>

namespace scene { namespace material {

constexpr float Dot_min = 0.00001f;


inline void Sample::Layer::set_tangent_frame(float3_p t, float3_p b, float3_p n) {
	t_ = t;
	b_ = b;
	n_ = n;
}

inline void Sample::Layer::set_tangent_frame(float3_p n) {
	math::orthonormal_basis(n, t_, b_);
	n_ = n;
}

inline float Sample::Layer::clamped_n_dot(float3_p v) const {
	// return std::max(math::dot(n, v), 0.00001f);
	return math::clamp(math::dot(n_, v), Dot_min, 1.f);
}

inline float Sample::Layer::reversed_clamped_n_dot(float3_p v) const {
	// return std::max(-math::dot(n, v), 0.00001f);
	return math::clamp(-math::dot(n_, v), Dot_min, 1.f);
}

inline float3_p Sample::Layer::shading_normal() const {
	return n_;
}

inline float3 Sample::Layer::tangent_to_world(float3_p v) const {
	return float3(v[0] * t_[0] + v[1] * b_[0] + v[2] * n_[0],
				  v[0] * t_[1] + v[1] * b_[1] + v[2] * n_[1],
				  v[0] * t_[2] + v[1] * b_[2] + v[2] * n_[2]);
}

inline float Sample::clamped_geo_n_dot(float3_p v) const {
	return math::clamp(math::dot(geo_n_, v), Dot_min, 1.f);
}

inline float Sample::reversed_clamped_geo_n_dot(float3_p v) const {
	return math::clamp(-math::dot(geo_n_, v), Dot_min, 1.f);
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
	float3 pushed = color + float3(0.01f, 0.01f, 0.01f);
	return float3(1.f / (pushed[0] * distance),
				  1.f / (pushed[1] * distance),
				  1.f / (pushed[2] * distance));
}

inline float Sample::clamped_dot(float3_p a, float3_p b) {
	return math::clamp(math::dot(a, b), Dot_min, 1.f);
}

inline float Sample::reversed_clamped_dot(float3_p a, float3_p b) {
	return math::clamp(-math::dot(a, b), Dot_min, 1.f);
}

inline float Sample::absolute_clamped_dot(float3_p a, float3_p b) {
	return math::clamp(std::abs(math::dot(a, b)), Dot_min, 1.f);
}

}}
