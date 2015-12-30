#pragma once

#include "triangle_bvh_builder_base.hpp"
#include "base/math/plane.hpp"
#include "base/math/vector.hpp"

namespace thread { class Pool; }

namespace scene { namespace shape { namespace triangle { namespace bvh {

class Builder_SAH : private Builder_base  {
public:

	Builder_SAH(uint32_t num_slices, uint32_t sweep_threshold);

	template<typename Data>
	void build(Tree<Data>& tree,
			   const std::vector<Index_triangle>& triangles,
			   const std::vector<Vertex>& vertices,
			   uint32_t max_primitives,
			   thread::Pool& thread_pool);

private:

	class Split_candidate {
	public:

		typedef std::vector<uint32_t>::iterator index;

		Split_candidate(uint8_t split_axis, const math::float3& p);

		void evaluate(index begin, index end,
					  float aabb_surface_area,
					  const std::vector<math::aabb>& triangle_bounds);

		float cost() const;

		bool behind(const math::float3& point) const;

		uint8_t axis() const;

		const math::aabb& aabb_0() const;
		const math::aabb& aabb_1() const;

	private:

		math::aabb aabb_0_;
		math::aabb aabb_1_;

		float d_;

		float cost_;

		uint8_t axis_;
	};

	template<typename Data>
	void split(Build_node* node,
			   index begin, index end,
			   const math::aabb& aabb,
			   const std::vector<Index_triangle>& triangles,
			   const std::vector<Vertex>& vertices,
			   const std::vector<math::aabb>& triangle_bounds,
			   uint32_t max_primitives,
			   thread::Pool& thread_pool,
			   Tree<Data>& tree);

	Split_candidate splitting_plane(index begin, index end,
									const math::aabb& aabb,
									const std::vector<math::aabb>& triangle_bounds,
									thread::Pool& thread_pool);

	std::vector<Split_candidate> split_candidates_;

	const uint32_t num_slices_;
	const uint32_t sweep_threshold_;
};

}}}}
