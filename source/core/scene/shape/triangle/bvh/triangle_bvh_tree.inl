#pragma once

#include "triangle_bvh_tree.hpp"
#include "triangle_bvh_node.inl"
#include "scene/scene_worker.hpp"
#include "scene/shape/node_stack.inl"
#include "scene/shape/triangle/triangle_intersection.hpp"
#include "base/math/vector.inl"
#include "base/math/bounding/aabb.inl"
#include "base/memory/align.inl"

namespace scene { namespace shape { namespace triangle { namespace bvh {

template<typename Data>
Tree<Data>::Tree() : num_nodes_(0), nodes_(nullptr), num_parts_(0), num_part_triangles_(nullptr) {}

template<typename Data>
Tree<Data>::~Tree() {
	delete [] num_part_triangles_;

	memory::free_aligned(nodes_);
}

template<typename Data>
Node* Tree<Data>::allocate_nodes(uint32_t num_nodes) {
	num_nodes_ = num_nodes;

	memory::free_aligned(nodes_);
	nodes_ = memory::allocate_aligned<Node>(num_nodes);

	return nodes_;
}

template<typename Data>
math::aabb Tree<Data>::aabb() const {
	return math::aabb(nodes_[0].bounds[0], nodes_[0].bounds[1]);
}

template<typename Data>
uint32_t Tree<Data>::num_parts() const {
	return num_parts_;
}

template<typename Data>
uint32_t Tree<Data>::num_triangles() const {
    return data_.num_triangles();
}

template<typename Data>
uint32_t Tree<Data>::num_triangles(uint32_t part) const {
	return num_part_triangles_[part];
}

template<typename Data>
uint32_t Tree<Data>::current_triangle() const {
	return data_.current_triangle();
}

template<typename Data>
bool Tree<Data>::intersect(math::Oray& ray, Node_stack& node_stack, Intersection& intersection) const {
	node_stack.clear();
	node_stack.push(0);
	uint32_t n = 0;

	math::float2 uv;
	uint32_t index = 0xFFFFFFFF;

	while (!node_stack.empty()) {
		auto& node = nodes_[n];

		if (node.intersect_p(ray)) {
			if (node.num_primitives > 0) {
				for (uint32_t i = node.primitive_offset, len = node.primitive_end(); i < len; ++i) {
					if (data_.intersect(i, ray, uv)) {
						index = i;
					}
				}

				n = node_stack.pop();
			} else {
				if (0 == ray.sign[node.axis]) {
					node_stack.push(node.second_child_index);
					n = n + 1;
				} else {
					node_stack.push(n + 1);
					n = node.second_child_index;
				}
			}
		} else {
			n = node_stack.pop();
		}
	}

	intersection.uv = uv;
	intersection.index = index;

	return index != 0xFFFFFFFF;
}

template<typename Data>
bool Tree<Data>::intersect_p(const math::Oray& ray, Node_stack& node_stack) const {
	node_stack.clear();
	node_stack.push(0);
	uint32_t n = 0;

	while (!node_stack.empty()) {
		auto& node = nodes_[n];

		if (node.intersect_p(ray)) {
			if (node.num_primitives > 0) {
				for (uint32_t i = node.primitive_offset, len = node.primitive_end(); i < len; ++i) {
					if (data_.intersect_p(i, ray)) {
						return true;
					}
				}

				n = node_stack.pop();
			} else {
				if (0 == ray.sign[node.axis]) {
					node_stack.push(node.second_child_index);
					n = n + 1;
				} else {
					node_stack.push(n + 1);
					n = node.second_child_index;
				}
			}
		} else {
			n = node_stack.pop();
		}
	}

	return false;
}

template<typename Data>
float Tree<Data>::opacity(math::Oray& ray, float time, const material::Materials& materials,
						  Worker& worker, material::Sampler_settings::Filter filter) const {
	auto& node_stack = worker.node_stack();
	node_stack.clear();
	node_stack.push(0);
	uint32_t n = 0;

	float opacity = 0.f;

	math::float2 uv;
	float max_t = ray.max_t;

	while (!node_stack.empty()) {
		auto& node = nodes_[n];

		if (node.intersect_p(ray)) {
			if (node.num_primitives > 0) {
				for (uint32_t i = node.primitive_offset, len = node.primitive_end(); i < len; ++i) {
					if (data_.intersect(i, ray, uv)) {
						uv = data_.interpolate_uv(i, uv);

						auto material = materials[data_.material_index(i)];

						opacity += (1.f - opacity) * material->opacity(uv, time, worker, filter);
						if (opacity >= 1.f) {
							return 1.f;
						}

						ray.max_t = max_t;
					}
				}

				n = node_stack.pop();
			} else {
				if (0 == ray.sign[node.axis]) {
					node_stack.push(node.second_child_index);
					n = n + 1;
				} else {
					node_stack.push(n + 1);
					n = node.second_child_index;
				}
			}
		} else {
			n = node_stack.pop();
		}
	}

	return opacity;
}

template<typename Data>
void Tree<Data>::interpolate_triangle_data(uint32_t index, math::float2 uv,
										   math::float3& n, math::float3& t, math::float2& tc) const {
	data_.interpolate_data(index, uv, n, t, tc);
}

template<typename Data>
math::float2 Tree<Data>::interpolate_triangle_uv(uint32_t index, math::float2 uv) const {
    return data_.interpolate_uv(index, uv);
}

template<typename Data>
float Tree<Data>::triangle_bitangent_sign(uint32_t index) const {
	return data_.bitangent_sign(index);
}

template<typename Data>
uint32_t Tree<Data>::triangle_material_index(uint32_t index) const {
    return data_.material_index(index);
}

template<typename Data>
math::float3 Tree<Data>::triangle_normal(uint32_t index) const {
    return data_.normal(index);
}

template<typename Data>
float Tree<Data>::triangle_area(uint32_t index) const {
	return data_.area(index);
}

template<typename Data>
float Tree<Data>::triangle_area(uint32_t index, const math::float3& scale) const {
    return data_.area(index, scale);
}

template<typename Data>
void Tree<Data>::sample(uint32_t index, math::float2 r2, math::float3& p, math::float3& n, math::float2& tc) const {
    data_.sample(index, r2, p, n, tc);
}

template<typename Data>
void Tree<Data>::sample(uint32_t index, math::float2 r2, math::float3& p, math::float2& tc) const {
	data_.sample(index, r2, p, tc);
}

template<typename Data>
void Tree<Data>::sample(uint32_t index, math::float2 r2, math::float3& p) const {
	data_.sample(index, r2, p);
}

template<typename Data>
void Tree<Data>::allocate_triangles(uint32_t num_triangles, uint32_t num_parts, const std::vector<Vertex>& vertices) {
	num_parts_ = num_parts;

	delete [] num_part_triangles_;
	num_part_triangles_ = new uint32_t[num_parts];

	for (uint32_t i = 0; i < num_parts; ++i) {
		num_part_triangles_[i] = 0;
	}

	data_.allocate_triangles(num_triangles, vertices);
}

template<typename Data>
void Tree<Data>::add_triangle(uint32_t a, uint32_t b, uint32_t c, uint32_t material_index,
							  const std::vector<Vertex>& vertices) {
	++num_part_triangles_[material_index];

	data_.add_triangle(a, b, c, material_index, vertices);
}

template<typename Data>
size_t Tree<Data>::num_bytes() const {
	return data_.num_bytes();
}

}}}}
