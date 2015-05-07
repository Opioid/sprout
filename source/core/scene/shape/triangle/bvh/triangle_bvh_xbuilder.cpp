#include "triangle_bvh_xbuilder.hpp"
#include "triangle_bvh_xtree.hpp"
#include "triangle_bvh_helper.hpp"
#include "scene/shape/triangle/triangle_primitive.hpp"
#include "base/math/vector.inl"
#include "base/math/plane.inl"
#include <iostream>

namespace scene { namespace shape { namespace triangle { namespace bvh {

XBuild_node::XBuild_node() : start_index(0), end_index(0) {
	children[0] = nullptr;
	children[1] = nullptr;
}

XBuild_node::~XBuild_node() {
	delete children[0];
	delete children[1];
}

void XBuild_node::num_sub_nodes(uint32_t& count) {
	if (children[0]) {
		count += 2;

		children[0]->num_sub_nodes(count);
		children[1]->num_sub_nodes(count);
	}
}

void XBuilder::build(XTree& tree, const std::vector<Index_triangle>& triangles, const std::vector<Vertex>& vertices, size_t max_primitives) {
	std::vector<uint32_t> primitive_indices(triangles.size());
	for (size_t i = 0, len = primitive_indices.size(); i < len; ++i) {
		primitive_indices[i] = static_cast<uint32_t>(i);
	}

	tree.allocate_triangles(static_cast<uint32_t>(triangles.size()));

	XBuild_node root;
	split(&root, primitive_indices, triangles, vertices, max_primitives, 0, tree);

	num_nodes_ = 1;
	root.num_sub_nodes(num_nodes_);

	nodes_ = &tree.allocate_nodes(num_nodes_);

	current_node_ = 0;
	serialize(&root);
}

void XBuilder::serialize(XBuild_node* node) {
	auto& n = new_node();
	n.aabb = node->aabb;
	n.start_index = node->start_index;
	n.end_index = node->end_index;
	n.axis = node->axis;

	if (node->children[0]) {
		serialize(node->children[0]);

		n.set_right_child(current_node_index());

		serialize(node->children[1]);

		n.set_has_children(true);
	}
}

XNode& XBuilder::new_node() {
	return (*nodes_)[current_node_++];
}

uint32_t XBuilder::current_node_index() const {
	return current_node_;
}

void XBuilder::split(XBuild_node* node,
					 const std::vector<uint32_t>& primitive_indices,
					 const std::vector<Index_triangle>& triangles,
					 const std::vector<Vertex>& vertices,
					 size_t max_primitives, uint32_t depth,
					 XTree& tree) {
	node->aabb = submesh_aabb(primitive_indices, triangles, vertices);

	if (primitive_indices.size() <= max_primitives || depth > 24) {
		assign(node, primitive_indices, triangles, vertices, tree);
	} else {
	//	math::plane sp = average_splitting_plane(node->aabb, primitive_indices, triangles, vertices, node->axis);

		Split_candidate sp = splitting_plane(node->aabb, primitive_indices, triangles, vertices);

		node->axis = sp.axis();

		size_t reserve_size = primitive_indices.size() / 2 + 1;
		std::vector<uint32_t> pids0;
		pids0.reserve(reserve_size);
		std::vector<uint32_t> pids1;
		pids1.reserve(reserve_size);

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
			node->children[0] = new XBuild_node;
			split(node->children[0], pids0, triangles, vertices, max_primitives, depth + 1, tree);

			node->children[1] = new XBuild_node;
			split(node->children[1], pids1, triangles, vertices, max_primitives, depth + 1, tree);
		}
	}
}

void XBuilder::assign(XBuild_node* node,
					  const std::vector<uint32_t>& primitive_indices,
					  const std::vector<Index_triangle>& triangles,
					  const std::vector<Vertex>& vertices,
					  XTree& tree) {
	node->start_index = tree.num_triangles();

	for (auto pi : primitive_indices) {
		auto& t = triangles[pi];
		tree.add_triangle(vertices[t.a], vertices[t.b], vertices[t.c], t.material_index);
	}

	node->end_index = tree.num_triangles();
}

math::AABB XBuilder::submesh_aabb(const std::vector<uint32_t>& primitive_indices, const std::vector<Index_triangle>& triangles, const std::vector<Vertex>& vertices) {
	float max_float = std::numeric_limits<float>::max();
	math::float3 min(max_float, max_float, max_float);
	math::float3 max(-max_float, -max_float, -max_float);

	for (auto pi : primitive_indices) {
		min = triangle_min(vertices[triangles[pi].a].p, vertices[triangles[pi].b].p, vertices[triangles[pi].c].p, min);
		max = triangle_max(vertices[triangles[pi].a].p, vertices[triangles[pi].b].p, vertices[triangles[pi].c].p, max);
	}

	const static float epsilon = 0.000000001f;

	max.x += epsilon;
	max.y += epsilon;
	max.z += epsilon;

	return math::AABB(min, max);
}

Split_candidate XBuilder::splitting_plane(const math::AABB& aabb,
										  const std::vector<uint32_t>& primitive_indices,
										  const std::vector<Index_triangle>& triangles,
										  const std::vector<Vertex>& vertices) {
	split_candidates_.clear();

	math::float3 average = math::float3::identity;

	for (auto pi : primitive_indices) {
		average += vertices[triangles[pi].a].p + vertices[triangles[pi].b].p + vertices[triangles[pi].c].p;
	}

	average /= static_cast<float>(primitive_indices.size() * 3);

//	math::float3 position = aabb.position();
	math::float3 halfsize = aabb.halfsize();

	uint8_t bb_axis;

	if (halfsize.x >= halfsize.y && halfsize.x >= halfsize.z) {
		bb_axis = 0;
	} else if (halfsize.y >= halfsize.x && halfsize.y >= halfsize.z) {
		bb_axis = 1;
	} else {
		bb_axis = 2;
	}

	split_candidates_.push_back(Split_candidate(bb_axis, 0, average,
								primitive_indices, triangles, vertices));
	split_candidates_.push_back(Split_candidate(bb_axis, 1, average,
								primitive_indices, triangles, vertices));
	split_candidates_.push_back(Split_candidate(bb_axis, 2, average,
								primitive_indices, triangles, vertices));

	std::sort(split_candidates_.begin(), split_candidates_.end(),
			  [](const Split_candidate& a, const Split_candidate& b){ return a.key() < b.key(); });

	return split_candidates_[0];
}

math::plane XBuilder::average_splitting_plane(const math::AABB& aabb,
											  const std::vector<uint32_t>& primitive_indices,
											  const std::vector<Index_triangle>& triangles,
											  const std::vector<Vertex>& vertices, uint8_t& axis) {
	math::float3 average = math::float3::identity;

	for (auto pi : primitive_indices) {
		average += vertices[triangles[pi].a].p + vertices[triangles[pi].b].p + vertices[triangles[pi].c].p;
	}

	average /= static_cast<float>(primitive_indices.size() * 3);

	math::float3 position = aabb.position();
	math::float3 halfsize = aabb.halfsize();

	if (halfsize.x >= halfsize.y && halfsize.x >= halfsize.z) {
		axis = 0;
		return math::plane(math::float3(1.f, 0.f, 0.f), math::float3(average.x, position.y, position.z));
	} else if (halfsize.y >= halfsize.x && halfsize.y >= halfsize.z) {
		axis = 1;
		return math::plane(math::float3(0.f, 1.f, 0.f), math::float3(position.x, average.y, position.z));
	} else {
		axis = 2;
		return math::plane(math::float3(0.f, 0.f, 1.f), math::float3(position.x, position.y, average.z));
	}
}

}}}}



