#pragma once

//#include "triangle_bvh_builder_base.hpp"
#include "base/math/plane.hpp"
#include "base/math/vector.hpp"
#include "base/math/bounding/aabb.hpp"
#include <vector>

namespace thread { class Pool; }

namespace scene { namespace shape {

struct Vertex;

namespace triangle {

struct Index_triangle;
struct Triangle;

namespace bvh {

template<typename Data> class Tree;
struct Node;

class Builder_SAH2 /*: private Builder_base*/  {

public:

	Builder_SAH2(uint32_t num_slices, uint32_t sweep_threshold);

	template<typename Data>
	void build(Tree<Data>& tree,
			   const std::vector<Index_triangle>& triangles,
			   const std::vector<Vertex>& vertices,
			   uint32_t num_parts,
			   uint32_t max_primitives,
			   thread::Pool& thread_pool);

private:

	struct Reference {
		math::aabb aabb;
		uint32_t   primitive;
	};

	using References = std::vector<Reference>;

	class Split_candidate {

	public:

		Split_candidate(uint8_t split_axis, float3_p p, bool spatial);

		void evaluate(const References& references, float aabb_surface_area);

		void distribute(const References& references,
						References& references0,
						References& references1) const;

		float cost() const;

		bool behind(float3_p point) const;

		uint8_t axis() const;
		bool spatial() const;

		const math::aabb& aabb_0() const;
		const math::aabb& aabb_1() const;

	private:

		math::aabb aabb_0_;
		math::aabb aabb_1_;

		float d_;

		float cost_;

		uint8_t axis_;

		bool spatial_;
	};

	struct Build_node {
		Build_node();
		~Build_node();

		math::aabb aabb;

		std::vector<uint32_t> primitives;

		uint32_t start_index;
		uint32_t end_index;

		uint8_t axis;

		Build_node* children[2];
	};

	void split(Build_node* node, const References& references,
			   const math::aabb& aabb, uint32_t max_primitives,
			   thread::Pool& thread_pool);

	Split_candidate splitting_plane(const References& references,
									const math::aabb& aabb,
									thread::Pool& thread_pool);

	template<typename Data>
	void serialize(Build_node* node,
				   const std::vector<Index_triangle>& triangles,
				   const std::vector<Vertex>& vertices,
				   Tree<Data>& tree);

	Node& new_node();

	uint32_t current_node_index() const;

	void assign(Build_node* node, const References& references);

	std::vector<Split_candidate> split_candidates_;

	uint32_t num_nodes_;
	uint32_t current_node_;

	uint32_t num_references_;

	Node* nodes_;

	const uint32_t num_slices_;
	const uint32_t sweep_threshold_;
};

}}}}
