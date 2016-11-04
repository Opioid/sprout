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

inline Node::Node() {}

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
inline bool Node::intersect_p(const math::Oray& ray) const {
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

inline bool Node::intersect_p(const math::Oray& ray, float& min_out, float& max_out) const {
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

}}}}
