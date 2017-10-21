#pragma once

#include "triangle_bvh_builder_base.hpp"
#include "triangle_bvh_tree.inl"
#include "triangle_bvh_helper.hpp"
#include "../triangle_primitive.hpp"

namespace scene::shape::triangle::bvh {

template<typename Data>
void Builder_base::assign(Build_node* node,
						  index begin, index end,
						  const std::vector<Index_triangle>& triangles,
						  const std::vector<Vertex>& vertices,
						  Tree<Data>& tree) {
	node->start_index = tree.current_triangle();

	for (index i = begin; i != end; ++i) {
		const auto& t = triangles[*i];
		tree.add_triangle(t.i[0], t.i[1], t.i[2], t.material_index, vertices);
	}

	node->end_index = tree.current_triangle();
}

}
