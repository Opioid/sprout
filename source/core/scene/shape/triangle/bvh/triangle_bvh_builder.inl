#pragma once

#include "triangle_bvh_builder.hpp"
#include "triangle_bvh_tree.inl"
#include "triangle_bvh_helper.hpp"
#include "scene/shape/node_stack.inl"
#include "scene/shape/triangle/triangle_primitive.hpp"
#include "base/math/vector.inl"
#include "base/math/plane.inl"
#include "base/math/bounding/aabb.inl"
#include <iostream>

namespace scene { namespace shape { namespace triangle { namespace bvh {

template<typename Data>
void Builder::build(Tree<Data>& tree, const std::vector<Index_triangle>& triangles, const std::vector<Vertex>& vertices, size_t max_primitives) {
	std::vector<uint32_t> primitive_indices(triangles.size());
	for (size_t i = 0, len = primitive_indices.size(); i < len; ++i) {
		primitive_indices[i] = static_cast<uint32_t>(i);
	}

	tree.allocate_triangles(static_cast<uint32_t>(triangles.size()));

	std::vector<uint32_t> swap_primitive_indices;
	swap_primitive_indices.reserve(triangles.size() / 2 + 1);

	Build_node root;
	split(&root, primitive_indices, swap_primitive_indices, triangles, vertices, max_primitives, 0, tree);

	num_nodes_ = 1;
	root.num_sub_nodes(num_nodes_);

	nodes_ = &tree.allocate_nodes(num_nodes_);

	current_node_ = 0;
	serialize(&root);
}

template<typename Data>
void Builder::split(Build_node* node,
					std::vector<uint32_t>& primitive_indices,
					std::vector<uint32_t>& swap_primitive_indices,
					const std::vector<Index_triangle>& triangles,
					const std::vector<Vertex>& vertices,
					size_t max_primitives, uint32_t depth,
                    Tree<Data>& tree) {
	node->aabb = submesh_aabb(primitive_indices, triangles, vertices);

	if (primitive_indices.size() <= max_primitives || depth > 16) {
		assign(node, primitive_indices, triangles, vertices, tree);
	} else {
		Split_candidate sp = splitting_plane(node->aabb, primitive_indices, triangles, vertices);

		node->axis = sp.axis();

		std::vector<uint32_t> pids0;
		pids0.swap(swap_primitive_indices);
		pids0.clear();

		std::vector<uint32_t> pids1;
		pids1.reserve(primitive_indices.size() / 2 + 1);

		for (auto pi : primitive_indices) {
			uint32_t side = triangle_side(vertices[triangles[pi].a].p, vertices[triangles[pi].b].p, vertices[triangles[pi].c].p, sp.plane());

			if (0 == side) {
				pids0.push_back(pi);
			} else {
				pids1.push_back(pi);
			}
		}

		if (pids0.empty()) {
			// This can happen if we didn't find a good splitting plane.
			// It means no triangle was completely on "this" side of the plane.
			assign(node, pids1, triangles, vertices, tree);
		} else {
			node->children[0] = new Build_node;
			split(node->children[0], pids0, primitive_indices, triangles, vertices, max_primitives, depth + 1, tree);

			node->children[1] = new Build_node;
			split(node->children[1], pids1, primitive_indices, triangles, vertices, max_primitives, depth + 1, tree);
		}
	}
}

template<typename Data>
void Builder::assign(Build_node* node,
					 const std::vector<uint32_t>& primitive_indices,
					 const std::vector<Index_triangle>& triangles,
					 const std::vector<Vertex>& vertices,
                     Tree<Data>& tree) {
	node->start_index = tree.num_triangles();

	for (auto pi : primitive_indices) {
		auto& t = triangles[pi];
		tree.add_triangle(vertices[t.a], vertices[t.b], vertices[t.c], t.material_index);
	}

	node->end_index = tree.num_triangles();
}

}}}}



