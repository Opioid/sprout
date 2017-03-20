#pragma once

#include "triangle_bvh_builder_sah2.hpp"
#include "triangle_bvh_builder_base.inl"
#include "triangle_bvh_tree.inl"
#include "triangle_bvh_helper.hpp"
#include "scene/shape/node_stack.hpp"
#include "scene/shape/triangle/triangle_primitive.hpp"
#include "base/math/aabb.inl"
#include "base/math/vector3.inl"
#include "base/math/plane.inl"
#include "base/math/simd_aabb.inl"
#include "base/thread/thread_pool.hpp"
#include <vector>

namespace scene { namespace shape { namespace triangle { namespace bvh {

template<typename Data>
void Builder_SAH2::build(Tree<Data>& tree, const Triangles& triangles, const Vertices& vertices,
						 uint32_t max_primitives, thread::Pool& thread_pool) {
	Build_node root;

	{
		References references(triangles.size());


		thread_pool.run_range([&triangles, &vertices, &references, &aabbs]
			(uint32_t id, int32_t begin, int32_t end) {
				for (int32_t i = begin; i < end; ++i) {
					auto a = math::load_float3(vertices[triangles[i].i[0]].p);
					auto b = math::load_float3(vertices[triangles[i].i[1]].p);
					auto c = math::load_float3(vertices[triangles[i].i[2]].p);

					auto min = triangle_min(a, b, c);
					auto max = triangle_max(a, b, c);

					references[i].set_min_max_primitive(min, max, i);

					aabb.merge_assign(min, max);
				}
				aabbs[id] = aabb;
			}, 0, static_cast<int32_t>(triangles.size()));

		for (auto& b : aabbs) {
			aabb.merge_assign(b);
		}

		num_nodes_ = 1;
		num_references_ = 0;

		split(&root, references, math::AABB(aabb.min, aabb.max), max_primitives, thread_pool);
	}

	tree.allocate_triangles(num_references_, vertices);

	nodes_ = tree.allocate_nodes(num_nodes_);

	current_node_ = 0;
	serialize(&root, triangles, vertices, tree);
}

template<typename Data>
void Builder_SAH2::serialize(Build_node* node, const Triangles& triangles,
							 const Vertices& vertices, Tree<Data>& tree) {
	auto& n = new_node();
	n.set_aabb(node->aabb.min().v, node->aabb.max().v);

	if (node->children[0]) {
		serialize(node->children[0], triangles, vertices, tree);

		n.set_split_node(current_node_index(), node->axis);

		serialize(node->children[1], triangles, vertices, tree);
	} else {
		uint8_t num_primitives = static_cast<uint8_t>(node->end_index - node->start_index);
		n.set_leaf_node(node->start_index, num_primitives);

		for (const auto p : node->primitives) {
			const auto& t = triangles[p];
			tree.add_triangle(t.i[0], t.i[1], t.i[2], t.material_index, vertices);
		}
	}
}

}}}}
