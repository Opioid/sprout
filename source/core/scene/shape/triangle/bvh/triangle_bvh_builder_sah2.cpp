#include "triangle_bvh_builder_sah2.hpp"
#include "triangle_bvh_tree.inl"
#include "triangle_bvh_helper.hpp"
#include "scene/shape/shape_vertex.hpp"
#include "scene/shape/triangle/triangle_primitive.hpp"
#include "base/math/aabb.inl"
#include "base/math/vector.inl"
#include "base/math/plane.inl"
#include "base/math/simd/simd_aabb.inl"
#include "base/thread/thread_pool.hpp"

namespace scene { namespace shape { namespace triangle { namespace bvh {

Builder_SAH2::Build_node::Build_node() : start_index(0), end_index(0) {
	children[0] = nullptr;
	children[1] = nullptr;
}

Builder_SAH2::Build_node::~Build_node() {
	delete children[0];
	delete children[1];
}

Builder_SAH2::Split_candidate::Split_candidate(uint8_t split_axis, float3_p p, bool spatial) :
	aabb_0_(math::aabb::empty()),
	aabb_1_(math::aabb::empty()),
	d_(p.v[split_axis]),
	axis_(split_axis),
	spatial_(spatial) {}

void Builder_SAH2::Split_candidate::evaluate(const References& references,
											 float aabb_surface_area) {
	uint32_t num_side_0 = 0;
	uint32_t num_side_1 = 0;

	math::simd::AABB box_0(aabb_0_);
	math::simd::AABB box_1(aabb_1_);

	if (spatial_) {
		for (const auto& r : references) {
			const auto& bounds = r.aabb;

			math::simd::AABB b(bounds);

			if (behind(bounds.max())) {
				++num_side_0;

			//	aabb_0_.merge_assign(bounds);
				box_0.merge_assign(b);
			} else if (!behind(bounds.min())) {
				++num_side_1;

			//	aabb_1_.merge_assign(bounds);
				box_1.merge_assign(b);
			} else {
				++num_side_0;
				++num_side_1;

			//	aabb_0_.merge_assign(bounds);
			//	aabb_1_.merge_assign(bounds);
				box_0.merge_assign(b);
				box_1.merge_assign(b);
			}
		}

		aabb_0_.set_min_max(box_0.min, box_0.max);
		aabb_1_.set_min_max(box_1.min, box_1.max);

		aabb_0_.clip_max(d_, axis_);
		aabb_1_.clip_min(d_, axis_);
	} else {
		for (const auto& r : references) {
			const auto& bounds = r.aabb;

			math::simd::AABB b(bounds);

			if (behind(bounds.max())) {
				++num_side_0;

			//	aabb_0_.merge_assign(bounds);
				box_0.merge_assign(b);
			} else {
				++num_side_1;

			//	aabb_1_.merge_assign(bounds);
				box_1.merge_assign(b);
			}
		}

		aabb_0_.set_min_max(box_0.min, box_0.max);
		aabb_1_.set_min_max(box_1.min, box_1.max);
	}

	if (0 == num_side_0 || 0 == num_side_1) {
		cost_ = 2.f + static_cast<float>(references.size());
	} else {
		cost_ = 2.f + (static_cast<float>(num_side_0) * aabb_0_.surface_area() +
					   static_cast<float>(num_side_1) * aabb_1_.surface_area()) / aabb_surface_area;
	}

	num_side_0_ = num_side_0;
	num_side_1_ = num_side_1;
}

void Builder_SAH2::Split_candidate::distribute(const References& references,
											   References& references0,
											   References& references1) const {
	references0.reserve(num_side_0_);
	references1.reserve(num_side_1_);

	if (spatial_) {
		for (const auto& r : references) {
			if (behind(r.aabb.max())) {
				references0.push_back(r);
			} else if (!behind(r.aabb.min())) {
				references1.push_back(r);
			} else {
				Reference r0 = r;
				r0.aabb.clip_max(d_, axis_);
				references0.push_back(r0);

				Reference r1 = r;
				r1.aabb.clip_min(d_, axis_);
				references1.push_back(r1);
			}
		}
	} else {
		for (const auto& r : references) {
			if (behind(r.aabb.max())) {
				references0.push_back(r);
			} else {
				references1.push_back(r);
			}
		}
	}
}

float Builder_SAH2::Split_candidate::cost() const {
	return cost_;
}

bool Builder_SAH2::Split_candidate::behind(float3_p point) const {
	return point.v[axis_] < d_;
}

uint8_t Builder_SAH2::Split_candidate::axis() const {
	return axis_;
}

bool Builder_SAH2::Split_candidate::spatial() const {
	return spatial_;
}

const math::aabb& Builder_SAH2::Split_candidate::aabb_0() const {
	return aabb_0_;
}

const math::aabb& Builder_SAH2::Split_candidate::aabb_1() const {
	return aabb_1_;
}

Builder_SAH2::Builder_SAH2(uint32_t num_slices, uint32_t sweep_threshold) :
	num_slices_(num_slices), sweep_threshold_(sweep_threshold) {}

void Builder_SAH2::split(Build_node* node, References& references, const math::aabb& aabb,
						 uint32_t max_primitives, thread::Pool& thread_pool) {
	node->aabb = aabb;

	uint32_t num_primitives = static_cast<uint32_t>(references.size());

	if (num_primitives <= max_primitives) {
		assign(node, references);
	} else {
		Split_candidate sp = splitting_plane(references, aabb, thread_pool);

		if (static_cast<float>(num_primitives) <= sp.cost()) {
			assign(node, references);
		} else {
			node->axis = sp.axis();

			References references0;
			References references1;
			sp.distribute(references, references0, references1);

			if (references0.empty() || references1.empty()) {
				// This can happen if we didn't find a good splitting plane.
				// It means every triangle was (partially) on the same side of the plane.

				assign(node, references);
			} else {
				references = References();

				node->children[0] = new Build_node;
				split(node->children[0], references0, sp.aabb_0(),
					  max_primitives, thread_pool);

				references0 = References();

				node->children[1] = new Build_node;
				split(node->children[1], references1, sp.aabb_1(),
					  max_primitives, thread_pool);

				num_nodes_ += 2;
			}
		}
	}
}

Builder_SAH2::Split_candidate Builder_SAH2::splitting_plane(const References& references,
															const math::aabb& aabb,
															thread::Pool& thread_pool) {
	static constexpr uint8_t X = 0;
	static constexpr uint8_t Y = 1;
	static constexpr uint8_t Z = 2;

	split_candidates_.clear();

	const uint32_t num_triangles = static_cast<uint32_t>(references.size());

	const float3 halfsize = aabb.halfsize();
	const float3 position = aabb.position();

	split_candidates_.emplace_back(X, position, true);
	split_candidates_.emplace_back(Y, position, true);
	split_candidates_.emplace_back(Z, position, true);

	if (num_triangles <= sweep_threshold_) {
		for (const auto& r : references) {
			float3_p max = r.aabb.max();
			split_candidates_.emplace_back(X, max, false);
			split_candidates_.emplace_back(Y, max, false);
			split_candidates_.emplace_back(Z, max, false);
		}
	} else {
		float3_p min = aabb.min();

		float3 step = (2.f * halfsize) / static_cast<float>(num_slices_);
		for (uint32_t i = 1, len = num_slices_; i < len; ++i) {
			const float fi = static_cast<float>(i);

			const float3 slice_x(min.v[0] + fi * step.v[0], position.v[1], position.v[2]);
			split_candidates_.emplace_back(X, slice_x, false);

			const float3 slice_y(position.v[0], min.v[1] + fi * step.v[1], position.v[2]);
			split_candidates_.emplace_back(Y, slice_y, false);

			const float3 slice_z(position.v[0], position.v[1], min.v[2] + fi * step.v[2]);
			split_candidates_.emplace_back(Z, slice_z, false);

//			split_candidates_.push_back(Split_candidate(0, slice_x, true));
//			split_candidates_.push_back(Split_candidate(1, slice_y, true));
//			split_candidates_.push_back(Split_candidate(2, slice_z, true));
		}
	}

	float aabb_surface_area = aabb.surface_area();

	// Arbitrary heuristic for starting the thread pool
	if (num_triangles < 1024) {
		for (auto& sc : split_candidates_) {
			sc.evaluate(references, aabb_surface_area);
		}
	} else {
		thread_pool.run_range([this, &references, aabb_surface_area]
			(uint32_t /*id*/, int32_t sc_begin, int32_t sc_end) {
				for (int32_t i = sc_begin; i < sc_end; ++i) {
					split_candidates_[i].evaluate(references, aabb_surface_area);
				}
			}, 0, static_cast<int32_t>(split_candidates_.size()));
	}

	size_t sc = 0;
	float  min_cost = split_candidates_[0].cost();

	for (size_t i = 1, len = split_candidates_.size(); i < len; ++i) {
		const float cost = split_candidates_[i].cost();
		if (cost < min_cost) {
			sc = i;
			min_cost = cost;
		}
	}

	return split_candidates_[sc];
}

Node& Builder_SAH2::new_node() {
	return nodes_[current_node_++];
}

uint32_t Builder_SAH2::current_node_index() const {
	return current_node_;
}

void Builder_SAH2::assign(Build_node* node, const References& references) {
	size_t num_references = references.size();
	node->primitives.resize(num_references);
	for (size_t i = 0; i < num_references; ++i) {
		node->primitives[i] = references[i].primitive;
	}

	node->start_index = num_references_;
	num_references_ += static_cast<uint32_t>(num_references);
	node->end_index = num_references_;
}

}}}}
