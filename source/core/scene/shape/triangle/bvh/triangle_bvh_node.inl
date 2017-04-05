#pragma once

#include "triangle_bvh_node.hpp"
#include "base/math/vector3.inl"
#include "base/simd/vector.inl"

namespace scene { namespace shape { namespace triangle { namespace bvh {

inline void Node::set_aabb(const float* min, const float* max) {
	min_.v[0] = min[0];
	min_.v[1] = min[1];
	min_.v[2] = min[2];

	max_.v[0] = max[0];
	max_.v[1] = max[1];
	max_.v[2] = max[2];
}

inline void Node::set_split_node(uint32_t next_node, uint8_t axis) {
	min_.next_or_data = next_node;
	max_.axis = axis;
	max_.num_primitives = 0;
}

inline void Node::set_leaf_node(uint32_t start_primitive, uint8_t num_primitives) {
	min_.next_or_data = start_primitive;
	max_.num_primitives = num_primitives;
}

// This test is presented in the paper
// "An Efficient and Robust Ray–Box Intersection Algorithm"
// http://www.cs.utah.edu/~awilliam/box/box.pdf
/*
inline bool Node::intersect_p(const math::Ray& ray) const {
	int8_t sign_0 = ray.signs[0];
	float min_t = (bounds[    sign_0].x - ray.origin.x) * ray.inv_direction.x;
	float max_t = (bounds[1 - sign_0].x - ray.origin.x) * ray.inv_direction.x;

	int8_t sign_1 = ray.signs[1];
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

	int8_t sign_2 = ray.signs[2];
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
}*/

// I found this SSE optimized AABB/ray test here:
// http://www.flipcode.com/archives/SSE_RayBox_Intersection_Test.shtml
inline bool Node::intersect_p(FVector ray_origin, FVector ray_inv_direction,
							  FVector ray_min_t, FVector ray_max_t) const {
	const Vector bb_min = load_float3(min_.v/*bounds[0]*/);
	const Vector bb_max = load_float3(max_.v/*bounds[1]*/);

	const Vector l1 = mul(sub(bb_min, ray_origin), ray_inv_direction);
	const Vector l2 = mul(sub(bb_max, ray_origin), ray_inv_direction);

	// the order we use for those min/max is vital to filter out
	// NaNs that happens when an inv_dir is +/- inf and
	// (box_min - pos) is 0. inf * 0 = NaN
	const Vector filtered_l1a = ::min(l1, simd::Infinity);
	const Vector filtered_l2a = ::min(l2, simd::Infinity);

	const Vector filtered_l1b = ::max(l1, simd::NegInfinity);
	const Vector filtered_l2b = ::max(l2, simd::NegInfinity);

	// now that we're back on our feet, test those slabs.
	Vector max_t = ::max(filtered_l1a, filtered_l2a);
	Vector min_t = ::min(filtered_l1b, filtered_l2b);

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
