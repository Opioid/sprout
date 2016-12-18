#pragma once

#include "aabb.hpp"
#include "math/vector3.inl"
#include <limits>

namespace math {

inline AABB::AABB(FVector3f_a min, FVector3f_a max) {
	bounds_[0] = min;
	bounds_[1] = max;
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
	return 2.f * (d.x * d.y + d.x * d.z + d.y * d.z);
}

inline float AABB::volume() const {
	Vector3f_a d = bounds_[1] - bounds_[0];
	return d.x * d.y * d.z;
}

inline bool AABB::intersect(FVector3f_a p) const {
	if (p.x >= bounds_[0].x && p.x <= bounds_[1].x
	&&  p.y >= bounds_[0].y && p.y <= bounds_[1].y
	&&  p.z >= bounds_[0].z && p.z <= bounds_[1].z) {
		return true;
	}

	return false;
}

// This test is presented in the paper
// "An Efficient and Robust Ray–Box Intersection Algorithm"
// http://www.cs.utah.edu/~awilliam/box/box.pdf
inline bool AABB::intersect_p(const math::Ray& ray) const {
	int8_t sign_0 = ray.sign[0];
	float min_t = (bounds_[    sign_0].x - ray.origin.x) * ray.inv_direction.x;
	float max_t = (bounds_[1 - sign_0].x - ray.origin.x) * ray.inv_direction.x;

	int8_t sign_1 = ray.sign[1];
	float min_ty = (bounds_[    sign_1].y - ray.origin.y) * ray.inv_direction.y;
	float max_ty = (bounds_[1 - sign_1].y - ray.origin.y) * ray.inv_direction.y;

	if (min_t > max_ty || min_ty > max_t) {
		return false;
	}

	if (min_ty > min_t) {
		min_t = min_ty;
	}

	if (max_ty < max_t) {
		max_t = max_ty;
	}

	int8_t sign_2 = ray.sign[2];
	float min_tz = (bounds_[    sign_2].z - ray.origin.z) * ray.inv_direction.z;
	float max_tz = (bounds_[1 - sign_2].z - ray.origin.z) * ray.inv_direction.z;

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

inline bool AABB::intersect_p(const math::Ray& ray, float& min_out, float& max_out) const {
	int8_t sign_0 = ray.sign[0];
	float min_t = (bounds_[    sign_0].x - ray.origin.x) * ray.inv_direction.x;
	float max_t = (bounds_[1 - sign_0].x - ray.origin.x) * ray.inv_direction.x;

	int8_t sign_1 = ray.sign[1];
	float min_ty = (bounds_[    sign_1].y - ray.origin.y) * ray.inv_direction.y;
	float max_ty = (bounds_[1 - sign_1].y - ray.origin.y) * ray.inv_direction.y;

	if (min_t > max_ty || min_ty > max_t) {
		return false;
	}

	if (min_ty > min_t) {
		min_t = min_ty;
	}

	if (max_ty < max_t) {
		max_t = max_ty;
	}

	int8_t sign_2 = ray.sign[2];
	float min_tz = (bounds_[    sign_2].z - ray.origin.z) * ray.inv_direction.z;
	float max_tz = (bounds_[1 - sign_2].z - ray.origin.z) * ray.inv_direction.z;

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

inline void AABB::insert(FVector3f_a p) {
	bounds_[0] = math::min(p, bounds_[0]);
	bounds_[1] = math::max(p, bounds_[1]);
}

inline AABB AABB::transform(const Matrix4x4f_a& m) const {
	Vector3f_a xa = bounds_[0].x * m.v3.x;
	Vector3f_a xb = bounds_[1].x * m.v3.x;

	Vector3f_a ya = bounds_[0].y * m.v3.y;
	Vector3f_a yb = bounds_[1].y * m.v3.y;

	Vector3f_a za = bounds_[0].z * m.v3.z;
	Vector3f_a zb = bounds_[1].z * m.v3.z;

	return AABB(math::min(xa, xb) + math::min(ya, yb) + math::min(za, zb) + m.v3.w,
				math::max(xa, xb) + math::max(ya, yb) + math::max(za, zb) + m.v3.w);
}

inline AABB AABB::merge(const AABB& other) const {
	return AABB(math::min(bounds_[0], other.bounds_[0]),
				math::max(bounds_[1], other.bounds_[1]));
}

inline void AABB::merge_assign(const AABB& other) {
	bounds_[0] = math::min(bounds_[0], other.bounds_[0]);
	bounds_[1] = math::max(bounds_[1], other.bounds_[1]);
}

inline AABB AABB::empty() {
	float max = std::numeric_limits<float>::max();
	return AABB(Vector3f_a(max), Vector3f_a(-max));
}

inline AABB AABB::infinite() {
	float max = 1000000.f;
	return AABB(Vector3f_a(-max), Vector3f_a(max));
}

}
