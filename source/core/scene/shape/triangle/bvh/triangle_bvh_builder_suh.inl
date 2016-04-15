#pragma once

#include "triangle_bvh_builder_suh.hpp"
#include "triangle_bvh_builder_base.inl"
#include "triangle_bvh_tree.inl"
#include "triangle_bvh_helper.hpp"
#include "scene/shape/node_stack.inl"
#include "scene/shape/triangle/triangle_primitive.hpp"
#include "base/math/vector.inl"
#include "base/math/plane.inl"
#include "base/math/bounding/aabb.inl"

namespace scene { namespace shape { namespace triangle { namespace bvh {

template<typename Data>
void Builder_SUH::build(Tree<Data>& tree,
						const std::vector<Index_triangle>& triangles,
						const std::vector<Vertex>& vertices,
						uint32_t num_parts,
						uint32_t max_primitives) {
	std::vector<uint32_t> primitive_indices(triangles.size());
	for (uint32_t i = 0, len = static_cast<uint32_t>(primitive_indices.size()); i < len; ++i) {
		primitive_indices[i] = i;
	}

	tree.allocate_triangles(static_cast<uint32_t>(triangles.size()), num_parts, vertices);

	Build_node root;
	split(&root,
		  primitive_indices.begin(), primitive_indices.end(),
		  triangles, vertices, max_primitives, 0, tree);

	num_nodes_ = 1;
	root.num_sub_nodes(num_nodes_);

	nodes_ = tree.allocate_nodes(num_nodes_);

	current_node_ = 0;
	serialize(&root);
}

template<typename Data>
void Builder_SUH::split(Build_node* node,
						index begin, index end,
						const std::vector<Index_triangle>& triangles,
						const std::vector<Vertex>& vertices,
						uint32_t max_primitives, uint32_t depth,
						Tree<Data>& tree) {
	node->aabb = submesh_aabb(begin, end, triangles, vertices);

	if (static_cast<uint32_t>(std::distance(begin, end)) <= max_primitives || depth > 20) {
		assign(node, begin, end, triangles, vertices, tree);
	} else {
		Split_candidate sp = splitting_plane(node->aabb, begin, end, triangles, vertices);

		node->axis = sp.axis();

		index pids1_begin = std::partition(begin, end,
			[&sp, &triangles, &vertices](uint32_t pi) {
				auto& t = triangles[pi];
				return sp.completely_behind(vertices[t.a].p, vertices[t.b].p, vertices[t.c].p);
			});

		if (begin == pids1_begin || end == pids1_begin) {
			// This can happen if we didn't find a good splitting plane.
			// It means every triangle was (partially) on the same side of the plane.

			assign(node, begin, end, triangles, vertices, tree);
		} else {
			node->children[0] = new Build_node;
			split(node->children[0], begin, pids1_begin, triangles,
				  vertices, max_primitives, depth + 1, tree);

			node->children[1] = new Build_node;
			split(node->children[1], pids1_begin, end, triangles,
				  vertices, max_primitives, depth + 1, tree);
		}
	}
}

}}}}
