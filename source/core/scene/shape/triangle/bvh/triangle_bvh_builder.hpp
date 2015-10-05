#pragma once

#include "split_candidate.hpp"
#include "base/math/bounding/aabb.hpp"
#include "base/math/plane.hpp"
#include <cstdint>
#include <cstddef>
#include <vector>

namespace scene { namespace shape {

struct Vertex;

namespace triangle {

struct Index_triangle;
struct Triangle;

namespace bvh {

template<typename Data> class Tree;
struct Node;

struct Build_node {
	Build_node();
	~Build_node();

	void num_sub_nodes(uint32_t& count);

	math::aabb aabb;

	uint32_t start_index;
	uint32_t end_index;

	uint8_t axis;

	Build_node* children[2];
};

class Builder  {
public:

    template<typename Data>
	void build(Tree<Data>& tree,
			   const std::vector<Index_triangle>& triangles,
			   const std::vector<Vertex>& vertices,
			   size_t max_primitives);

private:

	void serialize(Build_node* node);

	Node& new_node();

	uint32_t current_node_index() const;

	uint32_t num_nodes_;
	uint32_t current_node_;

	std::vector<Node>* nodes_;

	std::vector<Split_candidate> split_candidates_;

    template<typename Data>
	void split(Build_node* node,
			   std::vector<uint32_t>& primitive_indices,
			   std::vector<uint32_t>& swap_primitive_indices,
			   const std::vector<Index_triangle>& triangles,
			   const std::vector<Vertex>& vertices,
			   size_t max_primitives, uint32_t depth,
               Tree<Data>& tree);

    template<typename Data>
	static void assign(Build_node* node,
					   const std::vector<uint32_t>& primitive_indices,
					   const std::vector<Index_triangle>& triangles,
					   const std::vector<Vertex>& vertices,
                       Tree<Data>& tree);

	static math::aabb submesh_aabb(const std::vector<uint32_t>& primitive_indices,
								   const std::vector<Index_triangle>& triangles,
								   const std::vector<Vertex>& vertices);

	Split_candidate splitting_plane(const math::aabb& aabb,
									const std::vector<uint32_t>& primitive_indices,
									const std::vector<Index_triangle>& triangles,
									const std::vector<Vertex>& vertices);

	static math::plane average_splitting_plane(const math::aabb& aabb,
											   const std::vector<uint32_t>& primitive_indices,
											   const std::vector<Index_triangle>& triangles,
											   const std::vector<Vertex>& vertices, uint8_t& axis);
};

}}}}


