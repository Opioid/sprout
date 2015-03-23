#include "triangle_bvh_builder.hpp"
#include "triangle_bvh_tree.hpp"
#include "scene/shape/triangle/triangle_primitive.hpp"
#include "base/math/vector.inl"
#include "base/math/plane.inl"

namespace scene { namespace shape { namespace triangle { namespace bvh {

Build_node::Build_node() {
	children[0] = nullptr;
	children[1] = nullptr;
}

Build_node::~Build_node() {
	delete children[0];
	delete children[1];
}

void Build_node::num_sub_nodes(uint32_t& count) {
	if (children[0]) {
		count += 2;

		children[0]->num_sub_nodes(count);
		children[1]->num_sub_nodes(count);
	}
}

void Builder::build(Tree& tree, const std::vector<Index_triangle>& triangles, const std::vector<Vertex>& vertices, size_t max_primitives) {
	std::vector<uint32_t> primitive_indices(triangles.size());
	for (size_t i = 0, len = primitive_indices.size(); i < len; ++i) {
		primitive_indices[i] = static_cast<uint32_t>(i);
	}

	tree.triangles_.clear();
	tree.triangles_.reserve(triangles.size());

	Build_node root;
	split(&root, primitive_indices, triangles, vertices, max_primitives, 0, tree.triangles_);

	num_nodes_ = 1;
	root.num_sub_nodes(num_nodes_);

	nodes_ = &tree.allocate_nodes(num_nodes_);

	serialize(&root);
}

void Builder::serialize(Build_node* node) {
	auto& n = new_node();
	n.aabb = node->aabb;
	n.start_index = node->start_index;
	n.end_index = node->end_index;
	n.axis = node->axis;
}

Node& Builder::new_node() {
	return (*nodes_)[current_node_++];
}

uint32_t Builder::current_node_index() const {
	return current_node_;
}

uint32_t triangle_side(const math::float3& a, const math::float3& b, const math::float3& c, const math::plane& p) {
	uint32_t behind = 0;

	if (math::behind(p, a)) {
		++behind;
	}

	if (math::behind(p, b)) {
		++behind;
	}

	if (math::behind(p, c)) {
		++behind;
	}

	if (3 == behind) {
		return 0;
	} else if (0 == behind) {
		return 1;
	} else {
		return 2;
	}
}

void Builder::split(Build_node* node,
					const std::vector<uint32_t>& primitive_indices,
					const std::vector<Index_triangle>& triangles,
					const std::vector<Vertex>& vertices,
					size_t max_primitives, uint32_t depth,
					std::vector<Triangle>& out_triangles) {
	node->aabb = submesh_aabb(primitive_indices, triangles, vertices);

	if (primitive_indices.size() < max_primitives || depth > 18) {
		assign(node, primitive_indices, triangles, vertices, out_triangles);
	} else {
		math::plane sp = average_splitting_plane(node->aabb, primitive_indices, triangles, vertices, node->axis);

		size_t reserve_size = primitive_indices.size() / 2 + 1;
		std::vector<uint32_t> pids0;
		pids0.reserve(reserve_size);
		std::vector<uint32_t> pids1;
		pids1.reserve(reserve_size);

		for (auto pi : primitive_indices) {
			uint32_t side = triangle_side(vertices[triangles[pi].a].p, vertices[triangles[pi].b].p, vertices[triangles[pi].c].p, sp);

			if (0 == side) {
				pids0.push_back(pi);
			} else {
				pids1.push_back(pi);
			}
		}

		node->children[0] = new Build_node;
		split(node->children[0], pids0, triangles, vertices, max_primitives, depth + 1, out_triangles);

		node->children[1] = new Build_node;
		split(node->children[1], pids1, triangles, vertices, max_primitives, depth + 1, out_triangles);
	}
}

void Builder::assign(Build_node* node,
					 const std::vector<uint32_t>& primitive_indices,
					 const std::vector<Index_triangle>& triangles,
					 const std::vector<Vertex>& vertices,
					 std::vector<Triangle>& out_triangles) {
	node->start_index = static_cast<uint32_t>(out_triangles.size());

	for (auto pi : primitive_indices) {
		auto& t = triangles[pi];
		out_triangles.push_back(Triangle{vertices[t.a], vertices[t.b], vertices[t.c], t.material_index});
	}

	node->start_index = static_cast<uint32_t>(out_triangles.size());
}

math::float3 triangle_min(const math::float3& a, const math::float3& b, const math::float3& c, const math::float3& x) {
	return math::min(a, math::min(b, math::min(c, x)));
}

math::float3 triangle_max(const math::float3& a, const math::float3& b, const math::float3& c, const math::float3& x) {
	return math::max(a, math::max(b, math::max(c, x)));
}

math::AABB Builder::submesh_aabb(const std::vector<uint32_t>& primitive_indices, const std::vector<Index_triangle>& triangles, const std::vector<Vertex>& vertices) {
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

math::plane Builder::average_splitting_plane(const math::AABB aabb,
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



