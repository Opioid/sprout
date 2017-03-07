#pragma once

#include "aabb.hpp"
#include "math/vector3.inl"
#include "math/matrix4x4.inl"
#include "math/simd/simd_vector.inl"
#include <limits>

namespace math {

inline constexpr AABB::AABB(FVector3f_a min, FVector3f_a max) : bounds_{min, max} {}

inline AABB::AABB(simd::FVector min, simd::FVector max) {
	simd::store_float3_unsafe(bounds_[0], min);
	simd::store_float3_unsafe(bounds_[1], max);
}

inline FVector3f_a AABB::min() const {
	return bounds_[0];
}

inline FVector3f_a AABB::max() const {
	return bounds_[1];
}

inline Vector3f_a AABB::position() const {
	return 0.5f * (bounds_[0] + bounds_[1]);
}

inline Vector3f_a AABB::halfsize() const {
	return 0.5f * (bounds_[1] - bounds_[0]);
}

inline float AABB::surface_area() const {
	Vector3f_a d = bounds_[1] - bounds_[0];
	return 2.f * (d[0] * d[1] + d[0] * d[2] + d[1] * d[2]);
}

inline float AABB::volume() const {
	Vector3f_a d = bounds_[1] - bounds_[0];
	return d[0] * d[1] * d[2];
}

inline bool AABB::intersect(FVector3f_a p) const {
	if (p[0] >= bounds_[0][0] && p[0] <= bounds_[1][0]
	&&  p[1] >= bounds_[0][1] && p[1] <= bounds_[1][1]
	&&  p[2] >= bounds_[0][2] && p[2] <= bounds_[1][2]) {
		return true;
	}

	return false;
}

// This test is presented in the paper
// "An Efficient and Robust Ray–Box Intersection Algorithm"
// http://www.cs.utah.edu/~awilliam/box/box.pdf
inline bool AABB::intersect_p(const Ray& ray) const {
	int8_t sign_0 = ray.signs[0];
	float min_t = (bounds_[    sign_0][0] - ray.origin[0]) * ray.inv_direction[0];
	float max_t = (bounds_[1 - sign_0][0] - ray.origin[0]) * ray.inv_direction[0];

	int8_t sign_1 = ray.signs[1];
	float min_ty = (bounds_[    sign_1][1] - ray.origin[1]) * ray.inv_direction[1];
	float max_ty = (bounds_[1 - sign_1][1] - ray.origin[1]) * ray.inv_direction[1];

	if (min_t > max_ty || min_ty > max_t) {
		return false;
	}

	if (min_ty > min_t) {
		min_t = min_ty;
	}

	if (max_ty < max_t) {
		max_t = max_ty;
	}

	int8_t sign_2 = ray.signs[2];
	float min_tz = (bounds_[    sign_2][2] - ray.origin[2]) * ray.inv_direction[2];
	float max_tz = (bounds_[1 - sign_2][2] - ray.origin[2]) * ray.inv_direction[2];

	if (min_t > max_tz || min_tz > max_t) {
		return false;
	}

	if (min_tz > min_t) {
		min_t = min_tz;
	}

	if (max_tz < max_t) {
		max_t = max_tz;
	}

	return min_t < ray.max_t && max_t > ray.min_t;
}

inline bool AABB::intersect_p(const Ray& ray, float& min_out, float& max_out) const {
	int8_t sign_0 = ray.signs[0];
	float min_t = (bounds_[    sign_0][0] - ray.origin[0]) * ray.inv_direction[0];
	float max_t = (bounds_[1 - sign_0][0] - ray.origin[0]) * ray.inv_direction[0];

	int8_t sign_1 = ray.signs[1];
	float min_ty = (bounds_[    sign_1][1] - ray.origin[1]) * ray.inv_direction[1];
	float max_ty = (bounds_[1 - sign_1][1] - ray.origin[1]) * ray.inv_direction[1];

	if (min_t > max_ty || min_ty > max_t) {
		return false;
	}

	if (min_ty > min_t) {
		min_t = min_ty;
	}

	if (max_ty < max_t) {
		max_t = max_ty;
	}

	int8_t sign_2 = ray.signs[2];
	float min_tz = (bounds_[    sign_2][2] - ray.origin[2]) * ray.inv_direction[2];
	float max_tz = (bounds_[1 - sign_2][2] - ray.origin[2]) * ray.inv_direction[2];

	if (min_t > max_tz || min_tz > max_t) {
		return false;
	}

	if (min_tz > min_t) {
		min_t = min_tz;
	}

	if (max_tz < max_t) {
		max_t = max_tz;
	}

	min_out = min_t;
	max_out = max_t;

	if (min_out < ray.min_t) {
		min_out = ray.min_t;
	}

	if (max_out > ray.max_t) {
		max_out = ray.max_t;
	}

	return min_t < ray.max_t && max_t > ray.min_t;
}

inline void AABB::set_min_max(FVector3f_a min, FVector3f_a max) {
	bounds_[0] = min;
	bounds_[1] = max;
}

inline void AABB::set_min_max(simd::FVector min, simd::FVector max) {
	simd::store_float3_unsafe(bounds_[0], min);
	simd::store_float3_unsafe(bounds_[1], max);
}

inline void AABB::insert(FVector3f_a p) {
	bounds_[0] = math::min(p, bounds_[0]);
	bounds_[1] = math::max(p, bounds_[1]);
}

inline AABB AABB::transform(const Matrix4x4f_a& m) const {
	Vector3f_a mx = m.x();
	Vector3f_a xa = bounds_[0][0] * mx;
	Vector3f_a xb = bounds_[1][0] * mx;

	Vector3f_a my = m.y();
	Vector3f_a ya = bounds_[0][1] * my;
	Vector3f_a yb = bounds_[1][1] * my;

	Vector3f_a mz = m.z();
	Vector3f_a za = bounds_[0][2] * mz;
	Vector3f_a zb = bounds_[1][2] * mz;

	Vector3f_a mw = m.w();

	return AABB((math::min(xa, xb) + math::min(ya, yb)) + (math::min(za, zb) + mw),
				(math::max(xa, xb) + math::max(ya, yb)) + (math::max(za, zb) + mw));
}

inline AABB AABB::merge(const AABB& other) const {
	return AABB(math::min(bounds_[0], other.bounds_[0]),
				math::max(bounds_[1], other.bounds_[1]));
}

inline void AABB::merge_assign(const AABB& other) {
	bounds_[0] = math::min(bounds_[0], other.bounds_[0]);
	bounds_[1] = math::max(bounds_[1], other.bounds_[1]);
}

inline void AABB::clip_min(float d, uint8_t axis) {
	bounds_[0].v[axis] = std::max(d, bounds_[0][axis]);
}

inline void AABB::clip_max(float d, uint8_t axis) {
	bounds_[1].v[axis] = std::min(d, bounds_[1][axis]);
}

inline constexpr AABB AABB::empty() {
	constexpr float max = std::numeric_limits<float>::max();
	return AABB(Vector3f_a(max), Vector3f_a(-max));
}

inline constexpr AABB AABB::infinite() {
	constexpr float max = std::numeric_limits<float>::max();
	return AABB(Vector3f_a(-max), Vector3f_a(max));
}

}
