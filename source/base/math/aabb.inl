#ifndef SU_BASE_MATH_AABB_INL
#define SU_BASE_MATH_AABB_INL

#include "aabb.hpp"
#include "matrix4x4.inl"
#include "ray.hpp"
#include "simd_vector.inl"
#include "vector3.inl"

#include <limits>

namespace math {

inline constexpr AABB::AABB(f_float3 min, f_float3 max) : bounds_{min, max} {}

inline AABB::AABB(FVector min, FVector max) {
	simd::store_float4(bounds_[0].v, min);
	simd::store_float4(bounds_[1].v, max);
}

inline float3 const& AABB::min() const {
	return bounds_[0];
}

inline float3 const& AABB::max() const {
	return bounds_[1];
}

inline float3 AABB::position() const {
	return 0.5f * (bounds_[0] + bounds_[1]);
}

inline float3 AABB::halfsize() const {
	return 0.5f * (bounds_[1] - bounds_[0]);
}

inline float AABB::surface_area() const {
	float3 const d = bounds_[1] - bounds_[0];
	return 2.f * (d[0] * d[1] + d[0] * d[2] + d[1] * d[2]);
}

inline float AABB::volume() const {
	float3 const d = bounds_[1] - bounds_[0];
	return d[0] * d[1] * d[2];
}

inline bool AABB::intersect(f_float3 p) const {
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
inline bool AABB::intersect_p(Ray const& ray) const {
/*	int8_t sign_0 = ray.signs[0];
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

	return min_t < ray.max_t && max_t > ray.min_t;*/

	Vector ray_origin		 = simd::load_float4(ray.origin.v);
	Vector ray_inv_direction = simd::load_float4(ray.inv_direction.v);
	Vector ray_min_t		 = simd::load_float(&ray.min_t);
	Vector ray_max_t		 = simd::load_float(&ray.max_t);

	const Vector bb_min = simd::load_float4(bounds_[0].v);
	const Vector bb_max = simd::load_float4(bounds_[1].v);

	const Vector l1 = mul(sub(bb_min, ray_origin), ray_inv_direction);
	const Vector l2 = mul(sub(bb_max, ray_origin), ray_inv_direction);

	// the order we use for those min/max is vital to filter out
	// NaNs that happens when an inv_dir is +/- inf and
	// (box_min - pos) is 0. inf * 0 = NaN
	const Vector filtered_l1a = math::min(l1, simd::Infinity);
	const Vector filtered_l2a = math::min(l2, simd::Infinity);

	const Vector filtered_l1b = math::max(l1, simd::Neg_infinity);
	const Vector filtered_l2b = math::max(l2, simd::Neg_infinity);

	// now that we're back on our feet, test those slabs.
	Vector max_t = math::max(filtered_l1a, filtered_l2a);
	Vector min_t = math::min(filtered_l1b, filtered_l2b);

	// unfold back. try to hide the latency of the shufps & co.
	max_t = math::min1(max_t, SU_ROTATE_LEFT(max_t));
	min_t = math::max1(min_t, SU_ROTATE_LEFT(min_t));

	max_t = math::min1(max_t, SU_MUX_HIGH(max_t, max_t));
	min_t = math::max1(min_t, SU_MUX_HIGH(min_t, min_t));

	return 0 != (_mm_comige_ss(max_t, ray_min_t) &
				 _mm_comige_ss(ray_max_t, min_t) &
				 _mm_comige_ss(max_t, min_t));
}

inline bool AABB::intersect_p(FVector ray_origin, FVector ray_inv_direction,
							  FVector ray_min_t, FVector ray_max_t) const {
	const Vector bb_min = simd::load_float4(bounds_[0].v);
	const Vector bb_max = simd::load_float4(bounds_[1].v);

	const Vector l1 = mul(sub(bb_min, ray_origin), ray_inv_direction);
	const Vector l2 = mul(sub(bb_max, ray_origin), ray_inv_direction);

	// the order we use for those min/max is vital to filter out
	// NaNs that happens when an inv_dir is +/- inf and
	// (box_min - pos) is 0. inf * 0 = NaN
	const Vector filtered_l1a = math::min(l1, simd::Infinity);
	const Vector filtered_l2a = math::min(l2, simd::Infinity);

	const Vector filtered_l1b = math::max(l1, simd::Neg_infinity);
	const Vector filtered_l2b = math::max(l2, simd::Neg_infinity);

	// now that we're back on our feet, test those slabs.
	Vector max_t = math::max(filtered_l1a, filtered_l2a);
	Vector min_t = math::min(filtered_l1b, filtered_l2b);

	// unfold back. try to hide the latency of the shufps & co.
	max_t = math::min1(max_t, SU_ROTATE_LEFT(max_t));
	min_t = math::max1(min_t, SU_ROTATE_LEFT(min_t));

	max_t = math::min1(max_t, SU_MUX_HIGH(max_t, max_t));
	min_t = math::max1(min_t, SU_MUX_HIGH(min_t, min_t));

	return 0 != (_mm_comige_ss(max_t, ray_min_t) &
				 _mm_comige_ss(ray_max_t, min_t) &
				 _mm_comige_ss(max_t, min_t));
}

inline bool AABB::intersect_p(Ray const& ray, float& hit_t) const {
	Vector ray_origin		 = simd::load_float4(ray.origin.v);
	Vector ray_inv_direction = simd::load_float4(ray.inv_direction.v);
	Vector ray_min_t		 = simd::load_float(&ray.min_t);
	Vector ray_max_t		 = simd::load_float(&ray.max_t);

	const Vector bb_min = simd::load_float4(bounds_[0].v);
	const Vector bb_max = simd::load_float4(bounds_[1].v);

	const Vector l1 = mul(sub(bb_min, ray_origin), ray_inv_direction);
	const Vector l2 = mul(sub(bb_max, ray_origin), ray_inv_direction);

	// the order we use for those min/max is vital to filter out
	// NaNs that happens when an inv_dir is +/- inf and
	// (box_min - pos) is 0. inf * 0 = NaN
	const Vector filtered_l1a = math::min(l1, simd::Infinity);
	const Vector filtered_l2a = math::min(l2, simd::Infinity);

	const Vector filtered_l1b = math::max(l1, simd::Neg_infinity);
	const Vector filtered_l2b = math::max(l2, simd::Neg_infinity);

	// now that we're back on our feet, test those slabs.
	Vector max_t = math::max(filtered_l1a, filtered_l2a);
	Vector min_t = math::min(filtered_l1b, filtered_l2b);

	// unfold back. try to hide the latency of the shufps & co.
	max_t = math::min1(max_t, SU_ROTATE_LEFT(max_t));
	min_t = math::max1(min_t, SU_ROTATE_LEFT(min_t));

	max_t = math::min1(max_t, SU_MUX_HIGH(max_t, max_t));
	min_t = math::max1(min_t, SU_MUX_HIGH(min_t, min_t));

	float const min_out = simd::get_x(min_t);
	float const max_out = simd::get_x(max_t);

	if (min_out < ray.min_t) {
		hit_t = max_out;
	} else {
		hit_t = min_out;
	}

	return 0 != (_mm_comige_ss(max_t, ray_min_t) &
				 _mm_comige_ss(ray_max_t, min_t) &
				 _mm_comige_ss(max_t, min_t));
}

inline bool AABB::intersect_inside(Ray const& ray, float& hit_t) const {
	Vector ray_origin		 = simd::load_float4(ray.origin.v);
	Vector ray_inv_direction = simd::load_float4(ray.inv_direction.v);
	Vector ray_min_t		 = simd::load_float(&ray.min_t);
	Vector ray_max_t		 = simd::load_float(&ray.max_t);

	const Vector bb_min = simd::load_float4(bounds_[0].v);
	const Vector bb_max = simd::load_float4(bounds_[1].v);

	const Vector l1 = mul(sub(bb_min, ray_origin), ray_inv_direction);
	const Vector l2 = mul(sub(bb_max, ray_origin), ray_inv_direction);

	// the order we use for those min/max is vital to filter out
	// NaNs that happens when an inv_dir is +/- inf and
	// (box_min - pos) is 0. inf * 0 = NaN
	const Vector filtered_l1a = math::min(l1, simd::Infinity);
	const Vector filtered_l2a = math::min(l2, simd::Infinity);

	const Vector filtered_l1b = math::max(l1, simd::Neg_infinity);
	const Vector filtered_l2b = math::max(l2, simd::Neg_infinity);

	// now that we're back on our feet, test those slabs.
	Vector max_t = math::max(filtered_l1a, filtered_l2a);
	Vector min_t = math::min(filtered_l1b, filtered_l2b);

	// unfold back. try to hide the latency of the shufps & co.
	max_t = math::min1(max_t, SU_ROTATE_LEFT(max_t));
	min_t = math::max1(min_t, SU_ROTATE_LEFT(min_t));

	max_t = math::min1(max_t, SU_MUX_HIGH(max_t, max_t));
	min_t = math::max1(min_t, SU_MUX_HIGH(min_t, min_t));

	float const min_out = simd::get_x(min_t);
	float const max_out = simd::get_x(max_t);

	if (min_out < ray.min_t) {
		hit_t = max_out;
	} else {
		hit_t = min_out;
	}

	return 0 != (_mm_comige_ss(max_t, ray_min_t) &
				 _mm_comige_ss(ray_min_t, min_t) &
				 _mm_comige_ss(ray_max_t, min_t) &
				 _mm_comige_ss(max_t, min_t));
}

inline void AABB::set_min_max(float3 const& min, float3 const& max) {
	bounds_[0] = min;
	bounds_[1] = max;
}

inline void AABB::set_min_max(FVector min, FVector max) {
	simd::store_float4(bounds_[0].v, min);
	simd::store_float4(bounds_[1].v, max);
}

inline void AABB::insert(f_float3 p) {
	bounds_[0] = math::min(p, bounds_[0]);
	bounds_[1] = math::max(p, bounds_[1]);
}

inline AABB AABB::transform(const Matrix4x4f_a& m) const {
	float3 mx = m.x();
	float3 xa = bounds_[0][0] * mx;
	float3 xb = bounds_[1][0] * mx;

	float3 my = m.y();
	float3 ya = bounds_[0][1] * my;
	float3 yb = bounds_[1][1] * my;

	float3 mz = m.z();
	float3 za = bounds_[0][2] * mz;
	float3 zb = bounds_[1][2] * mz;

	float3 mw = m.w();

	return AABB((math::min(xa, xb) + math::min(ya, yb)) + (math::min(za, zb) + mw),
				(math::max(xa, xb) + math::max(ya, yb)) + (math::max(za, zb) + mw));
}

inline AABB AABB::merge(AABB const& other) const {
	return AABB(math::min(bounds_[0], other.bounds_[0]),
				math::max(bounds_[1], other.bounds_[1]));
}

inline void AABB::merge_assign(AABB const& other) {
	bounds_[0] = math::min(bounds_[0], other.bounds_[0]);
	bounds_[1] = math::max(bounds_[1], other.bounds_[1]);
}

inline void AABB::clip_min(float d, uint8_t axis) {
	bounds_[0].v[axis] = std::max(d, bounds_[0][axis]);
}

inline void AABB::clip_max(float d, uint8_t axis) {
	bounds_[1].v[axis] = std::min(d, bounds_[1][axis]);
}

inline bool AABB::operator==(AABB const& other) const {
	return bounds_[0] == other.bounds_[0] && bounds_[1] == other.bounds_[1];
}

inline constexpr AABB AABB::empty() {
	constexpr float max = std::numeric_limits<float>::max();
	return AABB(float3(max), float3(-max));
}

inline constexpr AABB AABB::infinite() {
	constexpr float max = std::numeric_limits<float>::max();
	return AABB(float3(-max), float3(max));
}

}

#endif
