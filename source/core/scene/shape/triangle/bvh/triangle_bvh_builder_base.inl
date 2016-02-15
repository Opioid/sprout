#pragma once

#include "triangle_bvh_builder_base.hpp"
#include "triangle_bvh_tree.inl"
#include "triangle_bvh_helper.hpp"
#include "scene/shape/node_stack.inl"
#include "scene/shape/triangle/triangle_primitive.hpp"
#include "base/math/vector.inl"
#include "base/math/plane.inl"
#include "base/math/bounding/aabb.inl"

namespace scene { namespace shape { namespace triangle { namespace bvh {

template<typename Data>
void Builder_base::assign(Build_node* node,
						  index begin, index end,
						  const std::vector<Index_triangle>& triangles,
						  const std::vector<Vertex>& vertices,
						  Tree<Data>& tree) {
	node->start_index = tree.current_triangle();

	for (index i = begin; i != end; ++i) {
		auto& t = triangles[*i];
		tree.add_triangle(vertices[t.a], vertices[t.b], vertices[t.c], t.material_index);
	}

	node->end_index = tree.current_triangle();
}

}}}}
