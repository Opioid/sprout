#include "triangle_bvh_builder_suh.hpp"
#include "triangle_bvh_tree.inl"
#include "triangle_bvh_helper.hpp"
#include "scene/shape/triangle/triangle_primitive.hpp"
#include "scene/shape/geometry/vertex.hpp"
#include "base/math/vector.inl"
#include "base/math/plane.inl"
#include "base/math/bounding/aabb.inl"

namespace scene { namespace shape { namespace triangle { namespace bvh {

Builder_SUH::Split_candidate::Split_candidate(uint8_t bb_axis, uint8_t split_axis, const math::vec3& p,
											  index begin, index end,
											  const std::vector<Index_triangle>& triangles,
											  const std::vector<Vertex>& vertices) :
	d_(p.v[split_axis]), axis_(split_axis)  {
	math::vec3 n;

	switch (split_axis) {
	default:
	case 0: n = math::vec3(1.f, 0.f, 0.f); break;
	case 1: n = math::vec3(0.f, 1.f, 0.f); break;
	case 2: n = math::vec3(0.f, 0.f, 1.f); break;
	}

	plane_ = math::create_plane(n, p);

	key_ = std::abs(static_cast<int>(bb_axis) - static_cast<int>(split_axis));

	int num_side_0 = 0;
	int num_side_1 = 0;
	uint32_t split = 0;
	for (index i = begin; i != end; ++i) {
		auto pi = *i;
		auto& a = vertices[triangles[pi].a].p;
		auto& b = vertices[triangles[pi].b].p;
		auto& c = vertices[triangles[pi].c].p;
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

uint32_t Builder_SUH::Split_candidate::side(const math::vec3& a, const math::vec3& b, const math::vec3& c) const {
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

bool Builder_SUH::Split_candidate::completely_behind(const math::vec3& a,
													 const math::vec3& b,
													 const math::vec3& c) const {
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

Builder_SUH::Split_candidate Builder_SUH::splitting_plane(const math::aabb& aabb,
														  index begin, index end,
														  const std::vector<Index_triangle>& triangles,
														  const std::vector<Vertex>& vertices) {
	split_candidates_.clear();

	math::vec3 average = math::vec3_identity;

	for (index i = begin; i != end; ++i) {
		auto& t = triangles[*i];
		average += vertices[t.a].p + vertices[t.b].p + vertices[t.c].p;
	}

	average /= static_cast<float>(std::distance(begin, end) * 3);

	math::vec3 halfsize = aabb.halfsize();

	uint8_t bb_axis;

	if (halfsize.x >= halfsize.y && halfsize.x >= halfsize.z) {
		bb_axis = 0;
	} else if (halfsize.y >= halfsize.x && halfsize.y >= halfsize.z) {
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
		std::vector<math::vec3> positions;
		positions.reserve(std::distance(begin, end));

		for (index i = begin; i != end; ++i) {
			auto& t = triangles[*i];
			positions.push_back(vertices[t.a].p);
			positions.push_back(vertices[t.b].p);
			positions.push_back(vertices[t.c].p);
		}

		size_t middle = positions.size() / 2;
		std::nth_element(positions.begin(), positions.begin() + middle, positions.end(),
						 [](const math::vec3& a, const math::vec3& b) { return a.x < b.x; });
		math::vec3 x_median = positions[middle];

		std::nth_element(positions.begin(), positions.begin() + middle, positions.end(),
						 [](const math::vec3& a, const math::vec3& b) { return a.y < b.y; });
		math::vec3 y_median = positions[middle];

		std::nth_element(positions.begin(), positions.begin() + middle, positions.end(),
						 [](const math::vec3& a, const math::vec3& b) { return a.z < b.z; });
		math::vec3 z_median = positions[middle];

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
