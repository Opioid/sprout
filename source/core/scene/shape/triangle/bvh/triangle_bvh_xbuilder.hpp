#pragma once

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

class XTree;
struct XNode;

struct XBuild_node {
	XBuild_node();
	~XBuild_node();

	void num_sub_nodes(uint32_t& count);

	math::AABB aabb;

	uint32_t start_index;
	uint32_t end_index;

	uint8_t axis;

	XBuild_node* children[2];
};

class XBuilder  {
public:

	void build(XTree& tree, const std::vector<Index_triangle>& triangles, const std::vector<Vertex>& vertices, size_t max_primitives);

private:

	void serialize(XBuild_node* node);

	XNode& new_node();

	uint32_t current_node_index() const;

	uint32_t num_nodes_;
	uint32_t current_node_;

	std::vector<XNode>* nodes_;

	static void split(XBuild_node* node,
					  const std::vector<uint32_t>& primitive_indices,
					  const std::vector<Index_triangle>& triangles,
					  const std::vector<Vertex>& vertices,
					  size_t max_primitives, uint32_t depth,
					  XTree& tree);

	static void assign(XBuild_node* node,
					   const std::vector<uint32_t>& primitive_indices,
					   const std::vector<Index_triangle>& triangles,
					   const std::vector<Vertex>& vertices,
					   XTree& tree);

	static math::AABB submesh_aabb(const std::vector<uint32_t>& primitive_indices,
								   const std::vector<Index_triangle>& triangles,
								   const std::vector<Vertex>& vertices);

	static math::plane average_splitting_plane(const math::AABB aabb,
											   const std::vector<uint32_t>& primitive_indices,
											   const std::vector<Index_triangle>& triangles,
											   const std::vector<Vertex>& vertices, uint8_t& axis);
};

}}}}


