#pragma once

#include "aabb.hpp"
#include "math/vector3.inl"
#include <limits>

namespace math {

inline AABB::AABB() {}

inline AABB::AABB(const Vector3f_a& min, const Vector3f_a& max) {
	bounds_[0] = min;
	bounds_[1] = max;
}

inline const Vector3f_a& AABB::min() const {
	return bounds_[0];
}

inline const Vector3f_a& AABB::max() const {
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

inline bool AABB::intersect_p(const math::Oray& ray) const {
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

inline bool AABB::intersect_p(const math::Oray& ray, float& min_out, float& max_out) const {
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

inline bool SU_CALLCONV AABB::intersect_p(simd::FVector origin, simd::FVector inv_direction, float min_t, float max_t) {
	// you may already have those values hanging around somewhere
/*	const __m128
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
	*/

	simd::Vector box_min = simd::load_vec3(bounds_[0]);
	simd::Vector box_max = simd::load_vec3(bounds_[1]);

	simd::Vector l0 = simd::mul3(simd::sub3(box_min, origin), inv_direction);
	simd::Vector l1 = simd::mul3(simd::sub3(box_max, origin), inv_direction);

	return false;
}

inline void AABB::set_min_max(const Vector3f_a& min, const Vector3f_a& max) {
	bounds_[0] = min;
	bounds_[1] = max;
}

inline void AABB::insert(const Vector3f_a& p) {
	bounds_[0] = math::min(p, bounds_[0]);
	bounds_[1] = math::max(p, bounds_[1]);
}

inline AABB AABB::transform(const Matrix4x4f_a& m) const {
	Vector3f_a xa = bounds_[0].x * m.x.xyz();
	Vector3f_a xb = bounds_[1].x * m.x.xyz();

	Vector3f_a ya = bounds_[0].y * m.y.xyz();
	Vector3f_a yb = bounds_[1].y * m.y.xyz();

	Vector3f_a za = bounds_[0].z * m.z.xyz();
	Vector3f_a zb = bounds_[1].z * m.z.xyz();

	return AABB(math::min(xa, xb) + math::min(ya, yb) + math::min(za, zb) + m.w.xyz(),
				math::max(xa, xb) + math::max(ya, yb) + math::max(za, zb) + m.w.xyz());
}

inline AABB AABB::merge(const AABB& other) const {
	return AABB(math::min(bounds_[0], other.bounds_[0]), math::max(bounds_[1], other.bounds_[1]));
}

inline void AABB::merge_assign(const AABB& other) {
	bounds_[0] = math::min(bounds_[0], other.bounds_[0]);
	bounds_[1] = math::max(bounds_[1], other.bounds_[1]);
}

inline AABB AABB::empty() {
	float max = std::numeric_limits<float>::max();
	return AABB(Vector3f_a(max, max, max), Vector3f_a(-max, -max, -max));
}

inline AABB AABB::infinite() {
	float max = 1000000.f;
	return AABB(Vector3f_a(-max, -max, -max), Vector3f_a(max, max, max));
}

}
