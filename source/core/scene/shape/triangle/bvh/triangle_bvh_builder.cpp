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

Build_node::Build_node() : start_index(0), end_index(0) {
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

void Builder::serialize(Build_node* node) {
	auto& n = new_node();
	n.aabb = node->aabb;
	n.start_index = node->start_index;
	n.end_index = node->end_index;

	if (node->children[0]) {
		serialize(node->children[0]);

		n.set_right_child(current_node_index());

		serialize(node->children[1]);

		n.set_has_children(true);

		// axis and start_index share the same memory, so only set this if node contains no triangles
		n.set_axis(node->axis);
	}
}

Node& Builder::new_node() {
	return (*nodes_)[current_node_++];
}

uint32_t Builder::current_node_index() const {
	return current_node_;
}

math::aabb Builder::submesh_aabb(const std::vector<uint32_t>& primitive_indices, const std::vector<Index_triangle>& triangles, const std::vector<Vertex>& vertices) {
	float max_float = std::numeric_limits<float>::max();
	math::float3 min( max_float,  max_float,  max_float);
	math::float3 max(-max_float, -max_float, -max_float);

	for (auto pi : primitive_indices) {
		min = triangle_min(vertices[triangles[pi].a].p, vertices[triangles[pi].b].p, vertices[triangles[pi].c].p, min);
		max = triangle_max(vertices[triangles[pi].a].p, vertices[triangles[pi].b].p, vertices[triangles[pi].c].p, max);
	}

	const static float epsilon = 0.000000001f;

	max.x += epsilon;
	max.y += epsilon;
	max.z += epsilon;

	return math::aabb(min, max);
}

Split_candidate Builder::splitting_plane(const math::aabb& aabb,
										 const std::vector<uint32_t>& primitive_indices,
										 const std::vector<Index_triangle>& triangles,
										 const std::vector<Vertex>& vertices) {
	split_candidates_.clear();

	math::float3 average = math::float3::identity;

//	std::vector<math::float3> positions;
//	positions.reserve(primitive_indices.size());

	for (auto pi : primitive_indices) {
		average += vertices[triangles[pi].a].p + vertices[triangles[pi].b].p + vertices[triangles[pi].c].p;

//		positions.push_back(vertices[triangles[pi].a].p);
//		positions.push_back(vertices[triangles[pi].b].p);
//		positions.push_back(vertices[triangles[pi].c].p);
	}

	average /= static_cast<float>(primitive_indices.size() * 3);

	math::float3 position = aabb.position();
	math::float3 halfsize = aabb.halfsize();

	uint8_t bb_axis;

//	std::sort(positions.begin(), positions.end(), [](const math::float3& a, const math::float3& b) { return a.x < b.x; } );
//	math::float3 x_median = positions[positions.size() / 2];

//	std::sort(positions.begin(), positions.end(), [](const math::float3& a, const math::float3& b) { return a.y < b.y; } );
//	math::float3 y_median = positions[positions.size() / 2];

//	std::sort(positions.begin(), positions.end(), [](const math::float3& a, const math::float3& b) { return a.z < b.z; } );
//	math::float3 z_median = positions[positions.size() / 2];

	if (halfsize.x >= halfsize.y && halfsize.x >= halfsize.z) {
		bb_axis = 0;
	} else if (halfsize.y >= halfsize.x && halfsize.y >= halfsize.z) {
		bb_axis = 1;
	} else {
		bb_axis = 2;
	}

	split_candidates_.push_back(Split_candidate(bb_axis, 0, average,
								primitive_indices, triangles, vertices));
//	split_candidates_.push_back(Split_candidate(bb_axis, 0, x_median,
//								primitive_indices, triangles, vertices));

	split_candidates_.push_back(Split_candidate(bb_axis, 1, average,
								primitive_indices, triangles, vertices));
//	split_candidates_.push_back(Split_candidate(bb_axis, 1, y_median,
//								primitive_indices, triangles, vertices));

	split_candidates_.push_back(Split_candidate(bb_axis, 2, average,
								primitive_indices, triangles, vertices));
//	split_candidates_.push_back(Split_candidate(bb_axis, 2, z_median,
//								primitive_indices, triangles, vertices));

/*
	math::float3 v = average - position;

	float modifier = 0.95f;

	split_candidates_.push_back(Split_candidate(bb_axis, 0, position + modifier * v,
								primitive_indices, triangles, vertices));
	split_candidates_.push_back(Split_candidate(bb_axis, 1, position + modifier * v,
								primitive_indices, triangles, vertices));
	split_candidates_.push_back(Split_candidate(bb_axis, 2, position + modifier * v,
								primitive_indices, triangles, vertices));

	split_candidates_.push_back(Split_candidate(bb_axis, 0, position - modifier * v,
								primitive_indices, triangles, vertices));
	split_candidates_.push_back(Split_candidate(bb_axis, 1, position - modifier * v,
								primitive_indices, triangles, vertices));
	split_candidates_.push_back(Split_candidate(bb_axis, 2, position - modifier * v,
								primitive_indices, triangles, vertices));
*/
	std::sort(split_candidates_.begin(), split_candidates_.end(),
			  [](const Split_candidate& a, const Split_candidate& b){ return a.key() < b.key(); });


	if (split_candidates_[0].key() >= 0x1000000000000000) {

			std::vector<math::float3> positions;
			positions.reserve(primitive_indices.size());

			for (auto pi : primitive_indices) {
				average += vertices[triangles[pi].a].p + vertices[triangles[pi].b].p + vertices[triangles[pi].c].p;

				positions.push_back(vertices[triangles[pi].a].p);
				positions.push_back(vertices[triangles[pi].b].p);
				positions.push_back(vertices[triangles[pi].c].p);
			}

				std::sort(positions.begin(), positions.end(), [](const math::float3& a, const math::float3& b) { return a.x < b.x; } );
				math::float3 x_median = positions[positions.size() / 2];

				std::sort(positions.begin(), positions.end(), [](const math::float3& a, const math::float3& b) { return a.y < b.y; } );
				math::float3 y_median = positions[positions.size() / 2];

				std::sort(positions.begin(), positions.end(), [](const math::float3& a, const math::float3& b) { return a.z < b.z; } );
				math::float3 z_median = positions[positions.size() / 2];

				split_candidates_.clear();

				split_candidates_.push_back(Split_candidate(bb_axis, 0, x_median,
											primitive_indices, triangles, vertices));
				split_candidates_.push_back(Split_candidate(bb_axis, 0, y_median,
											primitive_indices, triangles, vertices));
				split_candidates_.push_back(Split_candidate(bb_axis, 0, z_median,
											primitive_indices, triangles, vertices));

				split_candidates_.push_back(Split_candidate(bb_axis, 1, y_median,
											primitive_indices, triangles, vertices));
				split_candidates_.push_back(Split_candidate(bb_axis, 1, x_median,
											primitive_indices, triangles, vertices));
				split_candidates_.push_back(Split_candidate(bb_axis, 1, z_median,
											primitive_indices, triangles, vertices));

				split_candidates_.push_back(Split_candidate(bb_axis, 2, z_median,
											primitive_indices, triangles, vertices));
				split_candidates_.push_back(Split_candidate(bb_axis, 2, x_median,
											primitive_indices, triangles, vertices));
				split_candidates_.push_back(Split_candidate(bb_axis, 2, y_median,
											primitive_indices, triangles, vertices));


/*
				math::float3 v = average - position;

				float modifier = 0.f;

				split_candidates_.push_back(Split_candidate(bb_axis, 0, average + modifier * (average - x_median),
											primitive_indices, triangles, vertices));
				split_candidates_.push_back(Split_candidate(bb_axis, 1, average + modifier * (average - y_median),
											primitive_indices, triangles, vertices));
				split_candidates_.push_back(Split_candidate(bb_axis, 2, average + modifier * (average - z_median),
											primitive_indices, triangles, vertices));

				split_candidates_.push_back(Split_candidate(bb_axis, 0, average - modifier * (average - x_median),
											primitive_indices, triangles, vertices));
				split_candidates_.push_back(Split_candidate(bb_axis, 1, average - modifier * (average - y_median),
											primitive_indices, triangles, vertices));
				split_candidates_.push_back(Split_candidate(bb_axis, 2, average - modifier * (average - z_median),
											primitive_indices, triangles, vertices));
*/


				std::sort(split_candidates_.begin(), split_candidates_.end(),
						[](const Split_candidate& a, const Split_candidate& b){ return a.key() < b.key(); });
	}


	return split_candidates_[0];
}

math::plane Builder::average_splitting_plane(const math::aabb& aabb,
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



