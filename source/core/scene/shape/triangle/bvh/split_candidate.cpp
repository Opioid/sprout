#include "split_candidate.hpp"
#include "triangle_bvh_helper.hpp"
#include "scene/shape/triangle/triangle_primitive.hpp"
#include "base/math/bounding/aabb.hpp"
#include "base/math/plane.inl"

namespace scene { namespace shape { namespace triangle { namespace bvh {

Split_candidate::Split_candidate(const math::plane& plane, uint8_t axis) : plane_(plane), axis_(axis) {}

Split_candidate::Split_candidate(uint8_t bb_axis, uint8_t split_axis, const math::float3& p,
								 const std::vector<uint32_t>& primitive_indices,
								 const std::vector<Index_triangle>& triangles,
								 const std::vector<Vertex>& vertices) : axis_(split_axis) {
	math::float3 n;

	switch (split_axis) {
	default:
	case 0: n = math::float3(1.f, 0.f, 0.f); break;
	case 1: n = math::float3(0.f, 1.f, 0.f); break;
	case 2: n = math::float3(0.f, 0.f, 1.f); break;
	}

	plane_ = math::plane(n, p);

	key_ = std::abs(static_cast<int>(bb_axis) - static_cast<int>(split_axis));

//	math::AABB bb0 = math::AABB::empty();
//	math::AABB bb1 = math::AABB::empty();

	uint32_t num_side_0 = 0;
	uint32_t split = 0;
	for (auto pi : primitive_indices) {
		auto& a = vertices[triangles[pi].a].p;
		auto& b = vertices[triangles[pi].b].p;
		auto& c = vertices[triangles[pi].c].p;
		uint32_t side = triangle_side(a, b, c, plane_);

		if (0 == side) {
			++num_side_0;
//			bb0.insert(a);
//			bb0.insert(b);
//			bb0.insert(c);
		} else {
			if (2 == side) {
				++split;
			}

//			bb1.insert(a);
//			bb1.insert(b);
//			bb1.insert(c);
		}
	}

//	float volume = bb0.volume() + bb1.volume();
//	uint32_t volume_bits = *reinterpret_cast<uint32_t*>(&volume);

	key_ += split;

	if (0 == num_side_0) {
		key_ += 0x1000000000000000;
	}
}

uint64_t Split_candidate::key() const {
	return key_;
}

const math::plane& Split_candidate::plane() const {
	return plane_;
}

uint8_t Split_candidate::axis() const {
	return axis_;
}

}}}}
