#include "triangle_bvh_builder_suh.hpp"
#include "triangle_bvh_tree.inl"
#include "triangle_bvh_helper.hpp"
#include "scene/shape/shape_vertex.hpp"
#include "scene/shape/triangle/triangle_primitive.hpp"
#include "base/math/aabb.inl"
#include "base/math/vector.inl"
#include "base/math/plane.inl"

namespace scene { namespace shape { namespace triangle { namespace bvh {

Builder_SUH::Split_candidate::Split_candidate(uint8_t bb_axis, uint8_t split_axis,
											  float3_p p,
											  index begin, index end,
											  const std::vector<Index_triangle>& triangles,
											  const std::vector<Vertex>& vertices) :
	d_(p.v[split_axis]), axis_(split_axis)  {
	float3 n;

	switch (split_axis) {
	default:
	case 0: n = float3(1.f, 0.f, 0.f); break;
	case 1: n = float3(0.f, 1.f, 0.f); break;
	case 2: n = float3(0.f, 0.f, 1.f); break;
	}

	plane_ = math::create_plane(n, p);

	key_ = std::abs(static_cast<int>(bb_axis) - static_cast<int>(split_axis));

	int num_side_0 = 0;
	int num_side_1 = 0;
	uint32_t split = 0;
	for (index i = begin; i != end; ++i) {
		const auto pi = *i;
		const auto& a = vertices[triangles[pi].i[0]].p;
		const auto& b = vertices[triangles[pi].i[1]].p;
		const auto& c = vertices[triangles[pi].i[2]].p;
		uint32_t s = side(a, b, c);

		if (0 == s) {
			++num_side_0;
		} else {
			++num_side_1;

			if (2 == s) {
				++split;
			}
		}
	}

	key_ += split;

	if (0 == num_side_0) {
		key_ += 0x1000000000000000;
	}
}

uint64_t Builder_SUH::Split_candidate::key() const {
	return key_;
}

uint32_t Builder_SUH::Split_candidate::side(const math::packed_float3& a,
											const math::packed_float3& b,
											const math::packed_float3& c) const {
	uint32_t behind = 0;

	if (a.v[axis_] < d_) {
		++behind;
	}

	if (b.v[axis_] < d_) {
		++behind;
	}

	if (c.v[axis_] < d_) {
		++behind;
	}

	if (3 == behind) {
		return 0;
	} else if (0 == behind) {
		return 1;
	} else {
		return 2;
	}
}

bool Builder_SUH::Split_candidate::completely_behind(const math::packed_float3& a,
													 const math::packed_float3& b,
													 const math::packed_float3& c) const {
	if (a.v[axis_] < d_ && b.v[axis_] < d_ && c.v[axis_] < d_) {
		return true;
	}

	return false;
}

const math::plane& Builder_SUH::Split_candidate::plane() const {
	return plane_;
}

uint8_t Builder_SUH::Split_candidate::axis() const {
	return axis_;
}

Builder_SUH::Split_candidate Builder_SUH::splitting_plane(
		const math::aabb& aabb, index begin, index end,
		const std::vector<Index_triangle>& triangles, const std::vector<Vertex>& vertices) {
	split_candidates_.clear();

	float3 average = math::float3_identity;

	for (index i = begin; i != end; ++i) {
		const auto& t = triangles[*i];
		const auto a = float3(vertices[t.i[0]].p);
		const auto b = float3(vertices[t.i[1]].p);
		const auto c = float3(vertices[t.i[2]].p);
		average += a + b + c;
	}

	average /= static_cast<float>(std::distance(begin, end) * 3);

	float3 halfsize = aabb.halfsize();

	uint8_t bb_axis;

	if (halfsize.v[0] >= halfsize.v[1] && halfsize.v[0] >= halfsize.v[2]) {
		bb_axis = 0;
	} else if (halfsize.v[1] >= halfsize.v[0] && halfsize.v[1] >= halfsize.v[2]) {
		bb_axis = 1;
	} else {
		bb_axis = 2;
	}

	split_candidates_.push_back(Split_candidate(bb_axis, 0, average,
								begin, end, triangles, vertices));

	split_candidates_.push_back(Split_candidate(bb_axis, 1, average,
								begin, end, triangles, vertices));

	split_candidates_.push_back(Split_candidate(bb_axis, 2, average,
								begin, end, triangles, vertices));

	std::sort(split_candidates_.begin(), split_candidates_.end(),
		[](const Split_candidate& a, const Split_candidate& b){ return a.key() < b.key(); });

	if (split_candidates_[0].key() >= 0x1000000000000000) {
		std::vector<math::packed_float3> positions;
		positions.reserve(std::distance(begin, end));

		for (index i = begin; i != end; ++i) {
			const auto& t = triangles[*i];
			positions.push_back(vertices[t.i[0]].p);
			positions.push_back(vertices[t.i[1]].p);
			positions.push_back(vertices[t.i[2]].p);
		}

		size_t middle = positions.size() / 2;
		std::nth_element(positions.begin(), positions.begin() + middle, positions.end(),
						 [](const math::packed_float3& a, const math::packed_float3& b) {
								return a.x < b.x; });
		float3 x_median = float3(positions[middle]);

		std::nth_element(positions.begin(), positions.begin() + middle, positions.end(),
						 [](const math::packed_float3& a, const math::packed_float3& b) {
								return a.y < b.y; });
		float3 y_median = float3(positions[middle]);

		std::nth_element(positions.begin(), positions.begin() + middle, positions.end(),
						 [](const math::packed_float3& a, const math::packed_float3& b) {
								return a.z < b.z; });
		float3 z_median = float3(positions[middle]);

		split_candidates_.clear();

		split_candidates_.push_back(Split_candidate(bb_axis, 0, x_median,
									begin, end, triangles, vertices));
		split_candidates_.push_back(Split_candidate(bb_axis, 0, y_median,
									begin, end, triangles, vertices));
		split_candidates_.push_back(Split_candidate(bb_axis, 0, z_median,
									begin, end, triangles, vertices));

		split_candidates_.push_back(Split_candidate(bb_axis, 1, y_median,
									begin, end, triangles, vertices));
		split_candidates_.push_back(Split_candidate(bb_axis, 1, x_median,
									begin, end, triangles, vertices));
		split_candidates_.push_back(Split_candidate(bb_axis, 1, z_median,
									begin, end, triangles, vertices));

		split_candidates_.push_back(Split_candidate(bb_axis, 2, z_median,
									begin, end, triangles, vertices));
		split_candidates_.push_back(Split_candidate(bb_axis, 2, x_median,
									begin, end, triangles, vertices));
		split_candidates_.push_back(Split_candidate(bb_axis, 2, y_median,
									begin, end, triangles, vertices));

		std::sort(split_candidates_.begin(), split_candidates_.end(),
			[](const Split_candidate& a, const Split_candidate& b){ return a.key() < b.key(); });
	}

	size_t sc = 0;
	uint64_t  min_key = split_candidates_[0].key();

	for (size_t i = 1, len = split_candidates_.size(); i < len; ++i) {
		uint64_t key = split_candidates_[i].key();
		if (key < min_key) {
			sc = i;
			min_key = key;
		}
	}

	return split_candidates_[sc];
}

}}}}
