#include "triangle_bvh_builder_sah.hpp"
#include "triangle_bvh_tree.inl"
#include "triangle_bvh_helper.hpp"
#include "scene/bvh/scene_bvh_node.inl"
#include "scene/shape/shape_vertex.hpp"
#include "scene/shape/triangle/triangle_primitive.hpp"
#include "logging/logging.hpp"
#include "base/math/aabb.inl"
#include "base/math/vector3.inl"
#include "base/math/plane.inl"
#include "base/math/simd_aabb.inl"
#include "base/thread/thread_pool.hpp"

namespace scene::shape::triangle::bvh {

uint32_t Builder_SAH::Reference::primitive() const {
	return bounds[0].index;
}

void Builder_SAH::Reference::set_min_max_primitive(FVector min, FVector max, uint32_t primitive) {
	float3 tmp;
	simd::store_float4(tmp.v, min);
	bounds[0].v[0] = tmp[0]; bounds[0].v[1] = tmp[1]; bounds[0].v[2] = tmp[2];
	bounds[0].index = primitive;
	simd::store_float4(bounds[1].v, max);
}

void Builder_SAH::Reference::clip_min(float d, uint8_t axis) {
	bounds[0].v[axis] = std::max(d, bounds[0].v[axis]);
}

void Builder_SAH::Reference::clip_max(float d, uint8_t axis) {
	bounds[1].v[axis] = std::min(d, bounds[1].v[axis]);
}

Builder_SAH::Build_node::Build_node() :
	start_index(0),
	end_index(0),
	children{nullptr, nullptr} {}

Builder_SAH::Build_node::~Build_node() {
	delete children[0];
	delete children[1];
}

Builder_SAH::Split_candidate::Split_candidate(uint8_t split_axis, f_float3 p, bool spatial) :
	aabb_0_(math::AABB::empty()),
	aabb_1_(math::AABB::empty()),
	d_(p.v[split_axis]),
	axis_(split_axis),
	spatial_(spatial) {}

void Builder_SAH::Split_candidate::evaluate(const References& references, float aabb_surface_area) {
	uint32_t num_side_0 = 0;
	uint32_t num_side_1 = 0;

	math::Simd_AABB box_0(aabb_0_);
	math::Simd_AABB box_1(aabb_1_);

	if (spatial_) {
		for (const auto& r : references) {
			math::Simd_AABB b(r.bounds[0].v, r.bounds[1].v);

			if (behind(r.bounds[1].v)) {
				++num_side_0;

				box_0.merge_assign(b);
			} else if (!behind(r.bounds[0].v)) {
				++num_side_1;

				box_1.merge_assign(b);
			} else {
				++num_side_0;
				++num_side_1;

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
			math::Simd_AABB b(r.bounds[0].v, r.bounds[1].v);

			if (behind(r.bounds[1].v)) {
				++num_side_0;

				box_0.merge_assign(b);
			} else {
				++num_side_1;

				box_1.merge_assign(b);
			}
		}

		aabb_0_.set_min_max(box_0.min, box_0.max);
		aabb_1_.set_min_max(box_1.min, box_1.max);
	}

	const bool empty_side = 0 == num_side_0 || 0 == num_side_1;
	if (empty_side) {
		cost_ = 2.f + static_cast<float>(references.size());
	} else {
		const float weight_0 = static_cast<float>(num_side_0) * aabb_0_.surface_area();
		const float weight_1 = static_cast<float>(num_side_1) * aabb_1_.surface_area();
		cost_ = 2.f + (weight_0 + weight_1) / aabb_surface_area;
	}

	num_side_0_ = num_side_0;
	num_side_1_ = num_side_1;
}

void Builder_SAH::Split_candidate::distribute(const References& references,
											  References& references0,
											  References& references1) const {
	references0.reserve(num_side_0_);
	references1.reserve(num_side_1_);

	if (spatial_) {
		for (const auto& r : references) {
			if (behind(r.bounds[1].v)) {
				references0.push_back(r);
			} else if (!behind(r.bounds[0].v)) {
				references1.push_back(r);
			} else {
				Reference r0 = r;
				r0.clip_max(d_, axis_);
				references0.push_back(r0);

				Reference r1 = r;
				r1.clip_min(d_, axis_);
				references1.push_back(r1);
			}
		}
	} else {
		for (const auto& r : references) {
			if (behind(r.bounds[1].v)) {
				references0.push_back(r);
			} else {
				references1.push_back(r);
			}
		}
	}
}

float Builder_SAH::Split_candidate::cost() const {
	return cost_;
}

bool Builder_SAH::Split_candidate::behind(const float* point) const {
	return point[axis_] < d_;
}

uint8_t Builder_SAH::Split_candidate::axis() const {
	return axis_;
}

bool Builder_SAH::Split_candidate::spatial() const {
	return spatial_;
}

const math::AABB& Builder_SAH::Split_candidate::aabb_0() const {
	return aabb_0_;
}

const math::AABB& Builder_SAH::Split_candidate::aabb_1() const {
	return aabb_1_;
}

uint32_t Builder_SAH::Split_candidate::num_side_0() const {
	return num_side_0_;
}

uint32_t Builder_SAH::Split_candidate::num_side_1() const {
	return num_side_1_;
}

Builder_SAH::Builder_SAH(uint32_t num_slices, uint32_t sweep_threshold) :
	num_slices_(num_slices), sweep_threshold_(sweep_threshold) {}

void Builder_SAH::split(Build_node* node, References& references, const math::AABB& aabb,
						uint32_t max_primitives, uint32_t depth, thread::Pool& thread_pool) {
	node->aabb = aabb;

	const uint32_t num_primitives = static_cast<uint32_t>(references.size());

	if (num_primitives <= max_primitives) {
		assign(node, references);
	} else {
		bool exhausted;
		const Split_candidate sp = splitting_plane(references, aabb, depth, exhausted, thread_pool);

		if (num_primitives <= 0xFF
		&& (static_cast<float>(num_primitives) <= sp.cost() || exhausted)) {
			assign(node, references);
		} else {
			node->axis = sp.axis();

			References references0;
			References references1;
			sp.distribute(references, references0, references1);

			if (num_primitives <= 0xFF && (references0.empty() || references1.empty())) {
				// This can happen if we didn't find a good splitting plane.
				// It means every triangle was (partially) on the same side of the plane.
				assign(node, references);
			} else {
				if (exhausted) {
					// TODO
					// Implement a fallback solution that arbitrarily distributes the primitives
					// to sub-nodes without needing a meaningful splittng plane.
					logging::warning("Cannot split node further");
					return;
				}

				++depth;

				references = References();

				node->children[0] = new Build_node;
				split(node->children[0], references0, sp.aabb_0(),
					  max_primitives, depth, thread_pool);

				references0 = References();

				node->children[1] = new Build_node;
				split(node->children[1], references1, sp.aabb_1(),
					  max_primitives, depth, thread_pool);

				num_nodes_ += 2;
			}
		}
	}
}

Builder_SAH::Split_candidate Builder_SAH::splitting_plane(const References& references,
														  const math::AABB& aabb,
														  uint32_t depth, bool& exhausted,
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
			const float3& max(r.bounds[1].v);
			split_candidates_.emplace_back(X, max, false);
			split_candidates_.emplace_back(Y, max, false);
			split_candidates_.emplace_back(Z, max, false);
		}
	} else {
		const float3& min = aabb.min();

		const float3 step = (2.f * halfsize) / static_cast<float>(num_slices_);
		for (uint32_t i = 1, len = num_slices_; i < len; ++i) {
			const float fi = static_cast<float>(i);

			const float3 slice_x(min[0] + fi * step[0], position[1], position[2]);
			split_candidates_.emplace_back(X, slice_x, false);

			const float3 slice_y(position[0], min[1] + fi * step[1], position[2]);
			split_candidates_.emplace_back(Y, slice_y, false);

			const float3 slice_z(position[0], position[1], min[2] + fi * step[2]);
			split_candidates_.emplace_back(Z, slice_z, false);

			if (depth < spatial_split_threshold_) {
				split_candidates_.emplace_back(X, slice_x, true);
				split_candidates_.emplace_back(Y, slice_y, true);
				split_candidates_.emplace_back(Z, slice_z, true);
			}
		}
	}

	const float aabb_surface_area = aabb.surface_area();

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

	const auto& sp = split_candidates_[sc];

	exhausted = (sp.aabb_0() == aabb && num_triangles == sp.num_side_0()) ||
				(sp.aabb_1() == aabb && num_triangles == sp.num_side_1());

	return sp;
}

scene::bvh::Node& Builder_SAH::new_node() {
	return nodes_[current_node_++];
}

uint32_t Builder_SAH::current_node_index() const {
	return current_node_;
}

void Builder_SAH::assign(Build_node* node, const References& references) {
	const size_t num_references = references.size();
	node->primitives.resize(num_references);
	for (size_t i = 0; i < num_references; ++i) {
		node->primitives[i] = references[i].primitive();
	}

	node->start_index = num_references_;
	num_references_ += static_cast<uint32_t>(num_references);
	node->end_index = num_references_;
}

}
