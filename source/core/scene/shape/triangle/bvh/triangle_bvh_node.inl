#pragma once

#include "triangle_bvh_node.hpp"
#include "base/math/aabb.inl"
#include "base/math/vector.inl"

namespace scene { namespace shape { namespace triangle { namespace bvh {

inline void Node::set_aabb(const math::aabb& aabb) {
	bounds[0] = aabb.min();
	bounds[1] = aabb.max();
}

inline uint32_t Node::primitive_end() const {
	return next_or_data + static_cast<uint32_t>(num_primitives);
}

// This test is presented in the paper
// "An Efficient and Robust Ray–Box Intersection Algorithm"
// http://www.cs.utah.edu/~awilliam/box/box.pdf
inline bool Node::intersect_p(const math::Ray& ray) const {
	int8_t sign_0 = ray.sign[0];
	float min_t = (bounds[    sign_0].x - ray.origin.x) * ray.inv_direction.x;
	float max_t = (bounds[1 - sign_0].x - ray.origin.x) * ray.inv_direction.x;

	int8_t sign_1 = ray.sign[1];
	float min_ty = (bounds[    sign_1].y - ray.origin.y) * ray.inv_direction.y;
	float max_ty = (bounds[1 - sign_1].y - ray.origin.y) * ray.inv_direction.y;

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
	float min_tz = (bounds[    sign_2].z - ray.origin.z) * ray.inv_direction.z;
	float max_tz = (bounds[1 - sign_2].z - ray.origin.z) * ray.inv_direction.z;

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

// I found this SSE optimized AABB/ray test here:
// http://www.flipcode.com/archives/SSE_RayBox_Intersection_Test.shtml
inline bool Node::intersect_p(math::simd::FVector ray_origin,
							  math::simd::FVector ray_inv_direction,
							  math::simd::FVector ray_min_t,
							  math::simd::FVector ray_max_t) const {
	using namespace math::simd;

	const Vector bb_min = load_float3(bounds[0]);
	const Vector bb_max = load_float3(bounds[1]);

	const Vector l1 = mul3(sub3(bb_min, ray_origin), ray_inv_direction);
	const Vector l2 = mul3(sub3(bb_max, ray_origin), ray_inv_direction);

	// the order we use for those min/max is vital to filter out
	// NaNs that happens when an inv_dir is +/- inf and
	// (box_min - pos) is 0. inf * 0 = NaN
	const Vector filtered_l1a = min3(l1, Infinity);
	const Vector filtered_l2a = min3(l2, Infinity);

	const Vector filtered_l1b = max3(l1, NegInfinity);
	const Vector filtered_l2b = max3(l2, NegInfinity);

	// now that we're back on our feet, test those slabs.
	Vector max_t = max3(filtered_l1a, filtered_l2a);
	Vector min_t = min3(filtered_l1b, filtered_l2b);

	// unfold back. try to hide the latency of the shufps & co.
	max_t = min1(max_t, SU_ROTATE_LEFT(max_t));
	min_t = max1(min_t, SU_ROTATE_LEFT(min_t));

	max_t = min1(max_t, SU_MUX_HIGH(max_t, max_t));
	min_t = max1(min_t, SU_MUX_HIGH(min_t, min_t));

	return 0 != (_mm_comige_ss(max_t, ray_min_t) &
				 _mm_comige_ss(ray_max_t, min_t) &
				 _mm_comige_ss(max_t, min_t));
}

}}}}
