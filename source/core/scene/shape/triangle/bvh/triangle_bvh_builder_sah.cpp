#include "triangle_bvh_builder_sah.hpp"
#include "triangle_bvh_tree.inl"
#include "triangle_bvh_helper.hpp"
#include "scene/shape/shape_vertex.hpp"
#include "scene/shape/triangle/triangle_primitive.hpp"
#include "base/math/aabb.inl"
#include "base/math/vector3.inl"
#include "base/math/plane.inl"
#include "base/thread/thread_pool.hpp"

namespace scene { namespace shape { namespace triangle { namespace bvh {

Builder_SAH::Split_candidate::Split_candidate(uint8_t split_axis, float3_p p) :
	aabb_0_(math::AABB::empty()),
	aabb_1_(math::AABB::empty()),
	d_(p.v[split_axis]),
	axis_(split_axis) {}

void Builder_SAH::Split_candidate::evaluate(index begin, index end,
											float aabb_surface_area,
											aabbs triangle_bounds) {
	uint32_t num_side_0 = 0;
	uint32_t num_side_1 = 0;

	for (index i = begin; i != end; ++i) {
		const auto& bounds = triangle_bounds[*i];

		if (behind(bounds.max())) {
			++num_side_0;

			aabb_0_.merge_assign(bounds);
		} else {
			++num_side_1;

			aabb_1_.merge_assign(bounds);
		}
	}

	if (0 == num_side_0 || 0 == num_side_1) {
		cost_ = 2.f + static_cast<float>(std::distance(begin, end));
	} else {
		cost_ = 2.f + (static_cast<float>(num_side_0) * aabb_0_.surface_area() +
					   static_cast<float>(num_side_1) * aabb_1_.surface_area()) / aabb_surface_area;
	}
}

float Builder_SAH::Split_candidate::cost() const {
	return cost_;
}

bool Builder_SAH::Split_candidate::behind(float3_p point) const {
	return point.v[axis_] < d_;
}

uint8_t Builder_SAH::Split_candidate::axis() const {
	return axis_;
}

const math::AABB& Builder_SAH::Split_candidate::aabb_0() const {
	return aabb_0_;
}

const math::AABB& Builder_SAH::Split_candidate::aabb_1() const {
	return aabb_1_;
}

Builder_SAH::Builder_SAH(uint32_t num_slices, uint32_t sweep_threshold) :
	num_slices_(num_slices), sweep_threshold_(sweep_threshold) {}

Builder_SAH::Split_candidate Builder_SAH::splitting_plane(
		index begin, index end, const math::AABB& aabb,
		aabbs triangle_bounds, thread::Pool& thread_pool) {
	split_candidates_.clear();

	uint32_t num_triangles = static_cast<uint32_t>(std::distance(begin, end));

	if (num_triangles <= sweep_threshold_) {
		for (index i = begin; i != end; ++i) {
			float3_p max = triangle_bounds[*i].max();
			split_candidates_.push_back(Split_candidate(0, max));
			split_candidates_.push_back(Split_candidate(1, max));
			split_candidates_.push_back(Split_candidate(2, max));
		}
	} else {
		float3 halfsize = aabb.halfsize();
		float3 position = aabb.position();

		float3_p min = aabb.min();

		float3 step = (2.f * halfsize) / static_cast<float>(num_slices_);
		for (uint32_t i = 1, len = num_slices_; i < len; ++i) {
			float fi = static_cast<float>(i);

			float3 slice_x(min[0] + fi * step[0], position[1], position[2]);
			split_candidates_.push_back(Split_candidate(0, slice_x));

			float3 slice_y(position[0], min[1] + fi * step[1], position[2]);
			split_candidates_.push_back(Split_candidate(1, slice_y));

			float3 slice_z(position[0], position[1], min[2] + fi * step[2]);
			split_candidates_.push_back(Split_candidate(2, slice_z));
		}
	}

	float aabb_surface_area = aabb.surface_area();

	thread_pool.run_range(
		[this, begin, end, aabb_surface_area, &triangle_bounds]
		(uint32_t /*id*/, int32_t sc_begin, int32_t sc_end) {
			for (int32_t i = sc_begin; i < sc_end; ++i) {
				split_candidates_[i].evaluate(begin, end, aabb_surface_area, triangle_bounds);
			}
		}, 0, static_cast<int32_t>(split_candidates_.size()));

	size_t sc = 0;
	float  min_cost = split_candidates_[0].cost();

	for (size_t i = 1, len = split_candidates_.size(); i < len; ++i) {
		float cost = split_candidates_[i].cost();
		if (cost < min_cost) {
			sc = i;
			min_cost = cost;
		}
	}

	return split_candidates_[sc];
}

}}}}
