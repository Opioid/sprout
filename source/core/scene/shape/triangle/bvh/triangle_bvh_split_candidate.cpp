#include "triangle_bvh_split_candidate.hpp"
#include "triangle_bvh_helper.hpp"
#include "scene/shape/triangle/triangle_primitive.hpp"
#include "scene/shape/geometry/vertex.hpp"
#include "base/math/bounding/aabb.inl"
#include "base/math/plane.inl"

namespace scene { namespace shape { namespace triangle { namespace bvh {

Split_candidate::Split_candidate(const math::plane& plane, uint8_t axis) : plane_(plane), axis_(axis) {}

Split_candidate::Split_candidate(uint8_t bb_axis, uint8_t split_axis, const math::float3& p,
								 index begin, index end,
								 const std::vector<Index_triangle>& triangles,
								 const std::vector<Vertex>& vertices) : axis_(split_axis)  {
	math::float3 n;

	switch (split_axis) {
	default:
	case 0: n = math::float3(1.f, 0.f, 0.f); break;
	case 1: n = math::float3(0.f, 1.f, 0.f); break;
	case 2: n = math::float3(0.f, 0.f, 1.f); break;
	}

	plane_ = math::plane(n, p);

	key_ = std::abs(static_cast<int>(bb_axis) - static_cast<int>(split_axis));

	int num_side_0 = 0;
	int num_side_1 = 0;
	uint32_t split = 0;
	for (index i = begin; i != end; ++i) {
		auto pi = *i;
		auto& a = vertices[triangles[pi].a].p;
		auto& b = vertices[triangles[pi].b].p;
		auto& c = vertices[triangles[pi].c].p;
		uint32_t side = triangle_side(a, b, c, plane_);

		if (0 == side) {
			++num_side_0;
		} else {
			++num_side_1;

			if (2 == side) {
				++split;
			}
		}
	}

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
