#pragma once

#include "aabb.hpp"
#include "math/vector3.inl"
#include <limits>

namespace math {

template<typename T>
AABB<T>::AABB() {}

template<typename T>
AABB<T>::AABB(const Vector3<T>& min, const Vector3<T>& max) {
	bounds_[0] = min;
	bounds_[1] = max;
}

template<typename T>
const Vector3<T>& AABB<T>::min() const {
	return bounds_[0];
}

template<typename T>
const Vector3<T>& AABB<T>::max() const {
	return bounds_[1];
}

template<typename T>
Vector3<T> AABB<T>::position() const {
	return T(0.5) * (bounds_[0] + bounds_[1]);
}

template<typename T>
Vector3<T> AABB<T>::halfsize() const {
	return T(0.5) * (bounds_[1] - bounds_[0]);
}

template<typename T>
T AABB<T>::volume() const {
	Vector3<T> d = bounds_[1] - bounds_[0];
	return d.x * d.y * d.z;
}

template<typename T>
bool AABB<T>::intersect_p(const math::Optimized_ray<T>& ray) const {
	int sign_0 = ray.sign[0];
	T min_t = (bounds_[    sign_0].x - ray.origin.x) * ray.reciprocal_direction.x;
	T max_t = (bounds_[1 - sign_0].x - ray.origin.x) * ray.reciprocal_direction.x;

	int sign_1 = ray.sign[1];
	T min_ty = (bounds_[    sign_1].y - ray.origin.y) * ray.reciprocal_direction.y;
	T max_ty = (bounds_[1 - sign_1].y - ray.origin.y) * ray.reciprocal_direction.y;

	if (min_t > max_ty || min_ty > max_t) {
		return false;
	}

	if (min_ty > min_t) {
		min_t = min_ty;
	}

	if (max_ty < max_t) {
		max_t = max_ty;
	}

	int sign_2 = ray.sign[2];
	T min_tz = (bounds_[    sign_2].z - ray.origin.z) * ray.reciprocal_direction.z;
	T max_tz = (bounds_[1 - sign_2].z - ray.origin.z) * ray.reciprocal_direction.z;

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

template<typename T>
void AABB<T>::set_min_max(const Vector3<T>& min, const Vector3<T>& max) {
	bounds_[0] = min;
	bounds_[1] = max;
}

template<typename T>
void AABB<T>::insert(const Vector3<T>& p) {
	bounds_[0] = math::min(p, bounds_[0]);
	bounds_[1] = math::max(p, bounds_[1]);
}

template<typename T>
void AABB<T>::transform(const Matrix4x4<T>& m, AABB<T>& other) const {
	Vector3<T> xa = bounds_[0].x * m.x.xyz;
	Vector3<T> xb = bounds_[1].x * m.x.xyz;

	Vector3<T> ya = bounds_[0].y * m.y.xyz;
	Vector3<T> yb = bounds_[1].y * m.y.xyz;

	Vector3<T> za = bounds_[0].z * m.z.xyz;
	Vector3<T> zb = bounds_[1].z * m.z.xyz;

	other.bounds_[0] = math::min(xa, xb) + math::min(ya, yb) + math::min(za, zb) + m.w.xyz;
	other.bounds_[1] = math::max(xa, xb) + math::max(ya, yb) + math::max(za, zb) + m.w.xyz;
}

template<typename T>
AABB<T> AABB<T>::merge(const AABB<T>& other) const {
	return AABB<T>(math::min(bounds_[0], other.bounds_[0]), math::max(bounds_[1], other.bounds_[1]));
}

template<typename T>
AABB<T> AABB<T>::empty() {
	T max = std::numeric_limits<T>::max();
	return AABB<T>(Vector3<T>(max, max, max), Vector3<T>(-max, -max, -max));
}

}
