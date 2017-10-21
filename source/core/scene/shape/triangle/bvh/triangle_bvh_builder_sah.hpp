#pragma once

#include "triangle_bvh_builder_base.hpp"
#include "base/math/plane.hpp"
#include "base/math/vector3.hpp"

namespace thread { class Pool; }

namespace scene::shape::triangle::bvh {

class Builder_SAH : private Builder_base  {

public:

	Builder_SAH(uint32_t num_slices, uint32_t sweep_threshold);

	template<typename Data>
	void build(Tree<Data>& tree,
			   const std::vector<Index_triangle>& triangles,
			   const std::vector<Vertex>& vertices,
			   uint32_t num_parts,
			   uint32_t max_primitives,
			   thread::Pool& thread_pool);

private:

	using aabbs = const std::vector<math::AABB>&;

	class Split_candidate {

	public:

		using index = std::vector<uint32_t>::iterator;

		Split_candidate(uint8_t split_axis, const float3& p);

		void evaluate(index begin, index end, float aabb_surface_area,
					  aabbs triangle_bounds);

		float cost() const;

		bool behind(const float3& point) const;

		uint8_t axis() const;

		const math::AABB& aabb_0() const;
		const math::AABB& aabb_1() const;

	private:

		math::AABB aabb_0_;
		math::AABB aabb_1_;

		float d_;

		float cost_;

		uint8_t axis_;
	};

	template<typename Data>
	void split(Build_node* node,
			   index begin, index end,
			   const math::AABB& aabb,
			   const std::vector<Index_triangle>& triangles,
			   const std::vector<Vertex>& vertices,
			   aabbs triangle_bounds,
			   uint32_t max_primitives,
			   thread::Pool& thread_pool,
			   Tree<Data>& tree);

	Split_candidate splitting_plane(index begin, index end, const math::AABB& aabb,
									aabbs triangle_bounds, thread::Pool& thread_pool);

	std::vector<Split_candidate> split_candidates_;

	const uint32_t num_slices_;
	const uint32_t sweep_threshold_;
};

}
