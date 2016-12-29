#pragma once

#include "triangle_bvh_builder_sah2.hpp"
#include "triangle_bvh_builder_base.inl"
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
void Builder_SAH2::build(Tree<Data>& tree,
						const std::vector<Index_triangle>& triangles,
						const std::vector<Vertex>& vertices,
						uint32_t num_parts,
						uint32_t max_primitives,
						thread::Pool& thread_pool) {
	std::vector<uint32_t> primitive_indices(triangles.size());
	std::vector<math::aabb> primitive_bounds(triangles.size());

	std::vector<Reference> references(triangles.size());

	math::aabb aabb = math::aabb::empty();

	for (uint32_t i = 0, len = static_cast<uint32_t>(triangles.size()); i < len; ++i) {
		primitive_indices[i] = i;

		auto a = float3(vertices[triangles[i].a].p);
		auto b = float3(vertices[triangles[i].b].p);
		auto c = float3(vertices[triangles[i].c].p);

		float3 min = triangle_min(a, b, c);
		float3 max = triangle_max(a, b, c);

		primitive_bounds[i] = math::aabb(min, max);

		references[i].aabb = primitive_bounds[i];
		references[i].primitive = i;

		aabb.merge_assign(primitive_bounds[i]);
	}

	num_references_ = 0;

//	tree.allocate_triangles(static_cast<uint32_t>(triangles.size()), num_parts, vertices);

	Build_node root;
	split(&root, references, aabb, max_primitives, thread_pool);

	tree.allocate_triangles(num_references_, num_parts, vertices);

	num_nodes_ = 1;
	root.num_sub_nodes(num_nodes_);

	nodes_ = tree.allocate_nodes(num_nodes_);

	current_node_ = 0;
	serialize(&root, triangles, vertices, tree);
}

template<typename Data>
void Builder_SAH2::serialize(Build_node* node,
							 const std::vector<Index_triangle>& triangles,
							 const std::vector<Vertex>& vertices,
							 Tree<Data>& tree) {
	auto& n = new_node();
	n.set_aabb(node->aabb);

	if (node->children[0]) {
		serialize(node->children[0], triangles, vertices, tree);

		n.second_child_index = current_node_index();

		serialize(node->children[1], triangles, vertices, tree);

		n.axis = node->axis;
		n.num_primitives = 0;
	} else {
		n.primitive_offset = node->start_index;
		n.num_primitives = static_cast<uint8_t>(node->end_index - node->start_index);

		for (const auto& r : node->references) {
			const auto& t = triangles[r.primitive];
			tree.add_triangle(t.a, t.b, t.c, t.material_index, vertices);
		}
	}
}

}}}}
