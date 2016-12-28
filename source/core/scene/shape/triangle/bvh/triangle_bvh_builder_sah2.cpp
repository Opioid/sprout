#include "triangle_bvh_builder_sah2.hpp"
#include "triangle_bvh_tree.inl"
#include "triangle_bvh_helper.hpp"
#include "scene/shape/shape_vertex.hpp"
#include "scene/shape/triangle/triangle_primitive.hpp"
#include "base/math/vector.inl"
#include "base/math/plane.inl"
#include "base/math/bounding/aabb.inl"
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

void Builder_SAH2::Build_node::num_sub_nodes(uint32_t& count) {
	if (children[0]) {
		count += 2;

		children[0]->num_sub_nodes(count);
		children[1]->num_sub_nodes(count);
	}
}

Builder_SAH2::Split_candidate::Split_candidate(uint8_t split_axis, float3_p p, bool spatial) :
	aabb_0_(math::aabb::empty()),
	aabb_1_(math::aabb::empty()),
	d_(p.v[split_axis]),
	axis_(split_axis),
	spatial_(spatial) {}

void Builder_SAH2::Split_candidate::evaluate(index begin, index end,
											float aabb_surface_area,
											aabbs triangle_bounds) {
	uint32_t num_side_0 = 0;
	uint32_t num_side_1 = 0;

	for (index i = begin; i != end; ++i) {
		auto& bounds = triangle_bounds[*i];

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

float Builder_SAH2::Split_candidate::cost() const {
	return cost_;
}

bool Builder_SAH2::Split_candidate::behind(float3_p point) const {
	return point.v[axis_] < d_;
}

uint8_t Builder_SAH2::Split_candidate::axis() const {
	return axis_;
}

const math::aabb& Builder_SAH2::Split_candidate::aabb_0() const {
	return aabb_0_;
}

const math::aabb& Builder_SAH2::Split_candidate::aabb_1() const {
	return aabb_1_;
}

Builder_SAH2::Builder_SAH2(uint32_t num_slices, uint32_t sweep_threshold) :
	num_slices_(num_slices), sweep_threshold_(sweep_threshold) {}

void Builder_SAH2::split(Build_node* node, const std::vector<Reference>& references,
						 const math::aabb& aabb, uint32_t max_primitives) {
/*	node->aabb = aabb;

	uint32_t num_primitives = static_cast<uint32_t>(references.size());

	if (num_primitives <= max_primitives) {
		assign(node, references);
	} else {
		Split_candidate sp = splitting_plane(begin, end, aabb, triangle_bounds, thread_pool);

		if (static_cast<float>(num_primitives) <= sp.cost()) {
			assign(node, references);
		} else {
			node->axis = sp.axis();

			index pids1_begin = std::partition(begin, end,
				[&sp, &triangle_bounds](uint32_t pi) {
					return sp.behind(triangle_bounds[pi].max()); });

			if (begin == pids1_begin || end == pids1_begin) {
				// This can happen if we didn't find a good splitting plane.
				// It means every triangle was (partially) on the same side of the plane.

				assign(node, begin, end, triangles, vertices, tree);
			} else {
				node->children[0] = new Build_node;
				split(node->children[0], begin, pids1_begin, sp.aabb_0(),
					  triangles, vertices, triangle_bounds,
					  max_primitives, thread_pool, tree);

				node->children[1] = new Build_node;
				split(node->children[1], pids1_begin, end, sp.aabb_1(),
					  triangles, vertices, triangle_bounds,
					  max_primitives, thread_pool, tree);
			}
		}
	}
	*/
}

Builder_SAH2::Split_candidate Builder_SAH2::splitting_plane(
		index begin, index end, const math::aabb& aabb,
		aabbs triangle_bounds, thread::Pool& thread_pool) {
	split_candidates_.clear();

	uint32_t num_triangles = static_cast<uint32_t>(std::distance(begin, end));

	if (num_triangles <= sweep_threshold_) {
		for (index i = begin; i != end; ++i) {
			const float3& max = triangle_bounds[*i].max();
			split_candidates_.push_back(Split_candidate(0, max, false));
			split_candidates_.push_back(Split_candidate(1, max, false));
			split_candidates_.push_back(Split_candidate(2, max, false));
		}
	} else {
		float3 halfsize = aabb.halfsize();
		float3 position = aabb.position();

		float3 step = (2.f * halfsize) / static_cast<float>(num_slices_);
		for (uint32_t i = 1, len = num_slices_; i < len; ++i) {
			float fi = static_cast<float>(i);

			float3 slice_x(aabb.min().x + fi * step.x, position.y, position.z);
			split_candidates_.push_back(Split_candidate(0, slice_x, false));

			float3 slice_y(position.x, aabb.min().y + fi * step.y, position.z);
			split_candidates_.push_back(Split_candidate(1, slice_y, false));

			float3 slice_z(position.x, position.y, aabb.min().z + fi * step.z);
			split_candidates_.push_back(Split_candidate(2, slice_z, false));
		}
	}

	float aabb_surface_area = aabb.surface_area();

	thread_pool.run_range(
		[this, begin, end, aabb_surface_area, &triangle_bounds]
		(int32_t sc_begin, int32_t sc_end) {
			for (int32_t i = sc_begin; i < sc_end; ++i) {
				split_candidates_[i].evaluate(begin, end, aabb_surface_area, triangle_bounds);
			}
		},
		0, static_cast<int32_t>(split_candidates_.size()));

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

void Builder_SAH2::serialize(Build_node* node) {
	auto& n = new_node();
	n.set_aabb(node->aabb);

	if (node->children[0]) {
		serialize(node->children[0]);

		n.second_child_index = current_node_index();

		serialize(node->children[1]);

		n.axis = node->axis;
		n.num_primitives = 0;
	} else {
		n.primitive_offset = node->start_index;
		n.num_primitives = static_cast<uint8_t>(node->end_index - node->start_index);
	}
}

Node& Builder_SAH2::new_node() {
	return nodes_[current_node_++];
}

uint32_t Builder_SAH2::current_node_index() const {
	return current_node_;
}

void Builder_SAH2::assign(Build_node* node, const std::vector<Reference>& references) {
	node->references = references;
}

}}}}
