#pragma once

#include "material_sample.hpp"
#include "base/math/vector.inl"
#include <cmath>
//#include <algorithm>

namespace scene { namespace material {

//inline float Sample::absolute_n_dot_wo() const {
//	return std::abs(math::dot(n_, wo_));
//}

inline float Sample::clamped_n_dot_wo() const {
	return std::max(math::dot(n_, wo_), 0.00001f);
}

inline const math::vec3& Sample::shading_normal() const {
	return n_;
}

inline const math::vec3& Sample::geometric_normal() const {
	return geo_n_;
}

inline math::vec3 Sample::tangent_to_world(math::pvec3 v) const {
	return math::vec3(
		v.x * t_.x + v.y * b_.x + v.z * n_.x,
		v.x * t_.y + v.y * b_.y + v.z * n_.y,
		v.x * t_.z + v.y * b_.z + v.z * n_.z);
}

inline bool Sample::same_hemisphere(math::pvec3 v) const {
	return math::dot(geo_n_, v) > 0.f;
}

inline void Sample::set_basis(const math::vec3& t, const math::vec3& b, const math::vec3& n,
							  const math::vec3& geo_n, const math::vec3& wo, bool two_sided) {
	t_ = t;
	b_ = b;

	if (two_sided && math::dot(geo_n, wo) < 0.f) {
		n_ = -n;
		geo_n_ = -geo_n;
	} else {
		n_ = n;
		geo_n_ = geo_n;
	}

	wo_ = wo;
}

inline math::vec3 Sample::attenuation(math::pvec3 color, float distance) {
	return math::vec3(
		color.x > 0.f ? 1.f / (color.x * distance) : 0.f,
		color.y > 0.f ? 1.f / (color.y * distance) : 0.f,
		color.z > 0.f ? 1.f / (color.z * distance) : 0.f);
}

}}
