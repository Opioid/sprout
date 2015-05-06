#include "split_candidate.hpp"
#include "triangle_bvh_helper.hpp"
#include "scene/shape/triangle/triangle_primitive.hpp"
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

	uint32_t num_side_0 = 0;
	for (auto pi : primitive_indices) {
		uint32_t side = triangle_side(vertices[triangles[pi].a].p, vertices[triangles[pi].b].p, vertices[triangles[pi].c].p, plane_);

		if (0 == side) {
			++num_side_0;
		}
	}

	if (0 == num_side_0) {
		key_ += 100;
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
