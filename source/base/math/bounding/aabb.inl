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
T AABB<T>::surface_area() const {
	Vector3<T> d = bounds_[1] - bounds_[0];
	return T(2) * (d.x * d.y + d.x * d.z + d.y * d.z);
}

template<typename T>
T AABB<T>::volume() const {
	Vector3<T> d = bounds_[1] - bounds_[0];
	return d.x * d.y * d.z;
}

template<typename T>
bool AABB<T>::intersect_p(const math::Optimized_ray<T>& ray) const {
	int8_t sign_0 = ray.sign[0];
	T min_t = (bounds_[    sign_0].x - ray.origin.x) * ray.inv_direction.x;
	T max_t = (bounds_[1 - sign_0].x - ray.origin.x) * ray.inv_direction.x;

	int8_t sign_1 = ray.sign[1];
	T min_ty = (bounds_[    sign_1].y - ray.origin.y) * ray.inv_direction.y;
	T max_ty = (bounds_[1 - sign_1].y - ray.origin.y) * ray.inv_direction.y;

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
	T min_tz = (bounds_[    sign_2].z - ray.origin.z) * ray.inv_direction.z;
	T max_tz = (bounds_[1 - sign_2].z - ray.origin.z) * ray.inv_direction.z;

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
bool AABB<T>::intersect_p(const math::Optimized_ray<T>& ray, T& min_out, T& max_out) const {
	int8_t sign_0 = ray.sign[0];
	T min_t = (bounds_[    sign_0].x - ray.origin.x) * ray.inv_direction.x;
	T max_t = (bounds_[1 - sign_0].x - ray.origin.x) * ray.inv_direction.x;

	int8_t sign_1 = ray.sign[1];
	T min_ty = (bounds_[    sign_1].y - ray.origin.y) * ray.inv_direction.y;
	T max_ty = (bounds_[1 - sign_1].y - ray.origin.y) * ray.inv_direction.y;

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
	T min_tz = (bounds_[    sign_2].z - ray.origin.z) * ray.inv_direction.z;
	T max_tz = (bounds_[1 - sign_2].z - ray.origin.z) * ray.inv_direction.z;

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

/*
template<typename T>
bool AABB<T>::intersect_p(simd::FVector origin, simd::FVector inv_direction, float min_t, float max_t) {
	// you may already have those values hanging around somewhere
	const __m128
		plus_inf	= loadps(ps_cst_plus_inf),
		minus_inf	= loadps(ps_cst_minus_inf);

	// use whatever's apropriate to load.
	const __m128
		box_min	= loadps(&box.min),
		box_max	= loadps(&box.max),
		pos	= loadps(&ray.pos),
		inv_dir	= loadps(&ray.inv_dir);

	// use a div if inverted directions aren't available
	const __m128 l1 = mulps(subps(box_min, pos), inv_dir);
	const __m128 l2 = mulps(subps(box_max, pos), inv_dir);

	// the order we use for those min/max is vital to filter out
	// NaNs that happens when an inv_dir is +/- inf and
	// (box_min - pos) is 0. inf * 0 = NaN
	const __m128 filtered_l1a = minps(l1, plus_inf);
	const __m128 filtered_l2a = minps(l2, plus_inf);

	const __m128 filtered_l1b = maxps(l1, minus_inf);
	const __m128 filtered_l2b = maxps(l2, minus_inf);

	// now that we're back on our feet, test those slabs.
	__m128 lmax = maxps(filtered_l1a, filtered_l2a);
	__m128 lmin = minps(filtered_l1b, filtered_l2b);

	// unfold back. try to hide the latency of the shufps & co.
	const __m128 lmax0 = rotatelps(lmax);
	const __m128 lmin0 = rotatelps(lmin);
	lmax = minss(lmax, lmax0);
	lmin = maxss(lmin, lmin0);

	const __m128 lmax1 = muxhps(lmax,lmax);
	const __m128 lmin1 = muxhps(lmin,lmin);
	lmax = minss(lmax, lmax1);
	lmin = maxss(lmin, lmin1);

	const bool ret = _mm_comige_ss(lmax, _mm_setzero_ps()) & _mm_comige_ss(lmax,lmin);

	storess(lmin, &rs.t_near);
	storess(lmax, &rs.t_far);

	return  ret;

	simd::Vector box_min = simd::load_float3(bounds_[0]);
	simd::Vector box_max = simd::load_float3(bounds_[1]);

	simd::Vector l0 = simd::mul3(simd::sub3(box_min, origin), inv_direction);
	simd::Vector l1 = simd::mul3(simd::sub3(box_max, origin), inv_direction);

	// the order we use for those min/max is vital to filter out
	// NaNs that happens when an inv_dir is +/- inf and
	// (box_min - pos) is 0. inf * 0 = NaN
	const __m128 filtered_l1a = minps(l1, plus_inf);
	const __m128 filtered_l2a = minps(l2, plus_inf);

	const __m128 filtered_l1b = maxps(l1, minus_inf);
	const __m128 filtered_l2b = maxps(l2, minus_inf);

	return false;
}
*/

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
AABB<T> AABB<T>::transform(const Matrix4x4<T>& m) const {
	Vector3<T> xa = bounds_[0].x * m.x.xyz();
	Vector3<T> xb = bounds_[1].x * m.x.xyz();

	Vector3<T> ya = bounds_[0].y * m.y.xyz();
	Vector3<T> yb = bounds_[1].y * m.y.xyz();

	Vector3<T> za = bounds_[0].z * m.z.xyz();
	Vector3<T> zb = bounds_[1].z * m.z.xyz();

	return AABB<T>(math::min(xa, xb) + math::min(ya, yb) + math::min(za, zb) + m.w.xyz(),
				   math::max(xa, xb) + math::max(ya, yb) + math::max(za, zb) + m.w.xyz());
}

template<typename T>
AABB<T> AABB<T>::merge(const AABB<T>& other) const {
	return AABB<T>(math::min(bounds_[0], other.bounds_[0]), math::max(bounds_[1], other.bounds_[1]));
}

template<typename T>
void AABB<T>::merge_assign(const AABB& other) {
	bounds_[0] = math::min(bounds_[0], other.bounds_[0]);
	bounds_[1] = math::max(bounds_[1], other.bounds_[1]);
}

template<typename T>
AABB<T> AABB<T>::empty() {
	T max = std::numeric_limits<T>::max();
	return AABB<T>(Vector3<T>(max, max, max), Vector3<T>(-max, -max, -max));
}

template<typename T>
AABB<T> AABB<T>::infinite() {
	T max = T(1000000);
	return AABB<T>(Vector3<T>(-max, -max, -max), Vector3<T>(max, max, max));
}

}
