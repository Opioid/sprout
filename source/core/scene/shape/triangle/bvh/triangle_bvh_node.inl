#pragma once

#include "triangle_bvh_node.hpp"
#include "base/math/vector.inl"
#include "base/math/bounding/aabb.inl"

namespace scene { namespace shape { namespace triangle { namespace bvh {

/*
inline void Node::set_aabb(const math::aabb& aabb) {
	this->aabb = aabb;
}

inline uint32_t Node::primitive_end() const {
	return primitive_offset + static_cast<uint32_t>(num_primitives);
}
*/

inline void Node::set_aabb(const math::aabb& aabb) {
	bounds[0] = aabb.min();
	bounds[1] = aabb.max();
}

inline uint32_t Node::primitive_end() const {
	return primitive_offset + static_cast<uint32_t>(num_primitives);
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
inline bool Node::intersect_p(math::simd::FVector origin,
							  math::simd::FVector inv_direction,
							  math::simd::FVector min_t,
							  math::simd::FVector max_t) const {
	using namespace math::simd;

	const Vector box_min = load_float3(bounds[0]);
	const Vector box_max = load_float3(bounds[1]);

	const Vector l1 = mul3(sub3(box_min, origin), inv_direction);
	const Vector l2 = mul3(sub3(box_max, origin), inv_direction);

	// the order we use for those min/max is vital to filter out
	// NaNs that happens when an inv_dir is +/- inf and
	// (box_min - pos) is 0. inf * 0 = NaN
	const Vector filtered_l1a = min3(l1, Infinity);
	const Vector filtered_l2a = min3(l2, Infinity);

	const Vector filtered_l1b = max3(l1, NegInfinity);
	const Vector filtered_l2b = max3(l2, NegInfinity);

	// now that we're back on our feet, test those slabs.
	Vector lmax = max3(filtered_l1a, filtered_l2a);
	Vector lmin = min3(filtered_l1b, filtered_l2b);

	// unfold back. try to hide the latency of the shufps & co.
	lmax = min1(lmax, SU_ROTATE_LEFT(lmax));
	lmin = max1(lmin, SU_ROTATE_LEFT(lmin));

	lmax = min1(lmax, SU_MUX_HIGH(lmax, lmax));
	lmin = max1(lmin, SU_MUX_HIGH(lmin, lmin));

	return 0 != (_mm_comige_ss(lmax, min_t) &
				 _mm_comige_ss(max_t, lmin) &
				 _mm_comige_ss(lmax, lmin));
}

}}}}
