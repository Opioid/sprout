#pragma once

#include "base/math/aabb.hpp"
#include <cstdint>
#include <cstddef>
#include <vector>

namespace scene {

namespace bvh { class Node; }

namespace shape {

struct Vertex;

namespace triangle {

struct Index_triangle;
struct Triangle;

namespace bvh {

template<typename Data> class Tree;

struct Build_node {
	Build_node();
	~Build_node();

	void num_sub_nodes(uint32_t& count);

	math::AABB aabb;

	uint32_t start_index;
	uint32_t end_index;

	uint8_t axis;

	Build_node* children[2];
};

class Builder_base  {

protected:

	using Node = scene::bvh::Node;
	using index = std::vector<uint32_t>::iterator;

	void serialize(Build_node* node);

	Node& new_node();

	uint32_t current_node_index() const;

	template<typename Data>
	static void assign(Build_node* node,
					   index begin, index end,
					   const std::vector<Index_triangle>& triangles,
					   const std::vector<Vertex>& vertices,
					   Tree<Data>& tree);

	static math::AABB submesh_aabb(index begin, index end,
								   const std::vector<Index_triangle>& triangles,
								   const std::vector<Vertex>& vertices);

	uint32_t num_nodes_;
	uint32_t current_node_;

	Node* nodes_;
};

}}}}
