#pragma once

#include "material_sample.hpp"
#include "material_sample_helper.hpp"
#include "base/math/vector3.inl"
#include <cmath>

namespace scene { namespace material {

inline void Sample::Layer::set_tangent_frame(const float3& t, const float3& b, const float3& n) {
	t_ = t;
	b_ = b;
	n_ = n;
}

inline void Sample::Layer::set_tangent_frame(const float3& n) {
	math::orthonormal_basis(n, t_, b_);
	n_ = n;
}

inline float Sample::Layer::clamped_n_dot(const float3& v) const {
	// return std::max(math::dot(n, v), Dot_min);
	return clamped_dot(n_, v);
}

inline float Sample::Layer::reversed_clamped_n_dot(const float3& v) const {
	// return std::max(-math::dot(n, v), Dot_min);
	return reversed_clamped_dot(n_, v);
}

inline const float3& Sample::Layer::shading_normal() const {
	return n_;
}

inline float3 Sample::Layer::tangent_to_world(const float3& v) const {
	return float3(v[0] * t_[0] + v[1] * b_[0] + v[2] * n_[0],
				  v[0] * t_[1] + v[1] * b_[1] + v[2] * n_[1],
				  v[0] * t_[2] + v[1] * b_[2] + v[2] * n_[2]);
}

inline const float3& Sample::wo() const {
	return wo_;
}

inline float Sample::clamped_geo_n_dot(const float3& v) const {
	return clamped_dot(geo_n_, v);
}

inline float Sample::reversed_clamped_geo_n_dot(const float3& v) const {
	return reversed_clamped_dot(geo_n_, v);
}

inline const float3& Sample::geometric_normal() const {
	return geo_n_;
}

inline bool Sample::same_hemisphere(const float3& v) const {
	return math::dot(geo_n_, v) > 0.f;
}

inline void Sample::set_basis(const float3& geo_n, const float3& wo) {
	geo_n_ = geo_n;
	wo_    = wo;
}

}}
