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

// I found this SSE optimized version here
// http://www.flipcode.com/archives/SSE_RayBox_Intersection_Test.shtml

inline bool Node::intersect_p(math::simd::FVector origin,
							  math::simd::FVector inv_direction,
							  float min_t, float max_t) const {
	using namespace math;

	const simd::Vector box_min = simd::load_float3(bounds[0]);
	const simd::Vector box_max = simd::load_float3(bounds[1]);

	const simd::Vector l1 = simd::mul3(simd::sub3(box_min, origin), inv_direction);
	const simd::Vector l2 = simd::mul3(simd::sub3(box_max, origin), inv_direction);

	// the order we use for those min/max is vital to filter out
	// NaNs that happens when an inv_dir is +/- inf and
	// (box_min - pos) is 0. inf * 0 = NaN
	const simd::Vector filtered_l1a = simd::min3(l1, simd::Infinity);
	const simd::Vector filtered_l2a = simd::min3(l2, simd::Infinity);

	const simd::Vector filtered_l1b = simd::max3(l1, simd::NegInfinity);
	const simd::Vector filtered_l2b = simd::max3(l2, simd::NegInfinity);

	// now that we're back on our feet, test those slabs.
	simd::Vector lmax = simd::max3(filtered_l1a, filtered_l2a);
	simd::Vector lmin = simd::min3(filtered_l1b, filtered_l2b);

	// unfold back. try to hide the latency of the shufps & co.
	const simd::Vector lmax0 = SU_ROTATE_LEFT(lmax);
	const simd::Vector lmin0 = SU_ROTATE_LEFT(lmin);
	lmax = simd::min1(lmax, lmax0);
	lmin = simd::max1(lmin, lmin0);

	const simd::Vector lmax1 = SU_MUX_HIGH(lmax, lmax);
	const simd::Vector lmin1 = SU_MUX_HIGH(lmin, lmin);
	lmax = simd::min1(lmax, lmax1);
	lmin = simd::max1(lmin, lmin1);

	const simd::Vector ray_min_t = _mm_set1_ps(min_t);
	const simd::Vector ray_max_t = _mm_set1_ps(max_t);

	return 0 != (_mm_comige_ss(lmax, ray_min_t) &
				 _mm_comige_ss(ray_max_t, lmin) &
				 _mm_comige_ss(lmax, lmin));
}

}}}}
