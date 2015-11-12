#pragma once

#include "triangle_bvh_tree.hpp"
#include "scene/shape/node_stack.inl"
#include "scene/shape/triangle/triangle_intersection.hpp"
#include "base/math/vector.inl"
#include "base/math/bounding/aabb.inl"

namespace scene { namespace shape { namespace triangle { namespace bvh {

inline uint32_t Node::axis() const {
	return start_index & ~has_children_flag;
}

inline void Node::set_axis(uint32_t axis) {
	start_index |= axis;
}

inline bool Node::has_children() const {
	return has_children_flag == (start_index & has_children_flag);
}

inline void Node::set_has_children(bool children) {
	if (children) {
		start_index |= has_children_flag;
	} else {
		start_index &= ~has_children_flag;
	}
}

inline void Node::set_right_child(uint32_t offset) {
	end_index = offset;
}

template<typename Data>
std::vector<Node>& Tree<Data>::allocate_nodes(uint32_t num_nodes) {
    nodes_.resize(num_nodes);
    return nodes_;
}

template<typename Data>
const math::aabb& Tree<Data>::aabb() const {
	return nodes_[0].aabb;
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
bool Tree<Data>::intersect(math::Oray& ray, Node_stack& node_stack, Intersection& intersection) const {
	node_stack.clear();
	node_stack.push(0);
	uint32_t n = 0;

	math::float2 uv;
	uint32_t index = 0xFFFFFFFF;

	while (!node_stack.empty()) {
		auto& node = nodes_[n];

		if (node.aabb.intersect_p(ray)) {
			if (node.has_children()) {
				if (0 == ray.sign[node.axis()]) {
					node_stack.push(node.end_index);
					n = n + 1;
				} else {
					node_stack.push(n + 1);
					n = node.end_index;
				}
			} else {
				for (uint32_t i = node.start_index; i < node.end_index; ++i) {
                    if (data_.intersect(i, ray, uv)) {
						index = i;
					}
				}

				n = node_stack.pop();
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

		if (node.aabb.intersect_p(ray)) {
			if (node.has_children()) {
				if (0 == ray.sign[node.axis()]) {
					node_stack.push(node.end_index);
					n = n + 1;
				} else {
					node_stack.push(n + 1);
					n = node.end_index;
				}
			} else {
				for (uint32_t i = node.start_index; i < node.end_index; ++i) {
                    if (data_.intersect_p(i, ray)) {
						return true;
					}
				}

				n = node_stack.pop();
			}
		} else {
			n = node_stack.pop();
		}
	}

	return false;
}

template<typename Data>
float Tree<Data>::opacity(math::Oray& ray, Node_stack& node_stack,
						  const material::Materials& materials,
						  const image::texture::sampler::Sampler_2D& sampler) const {
	node_stack.clear();
	node_stack.push(0);
	uint32_t n = 0;

	float opacity = 0.f;

	math::float2 uv;
	float max_t = ray.max_t;

	while (!node_stack.empty()) {
		auto& node = nodes_[n];

		if (node.aabb.intersect_p(ray)) {
			if (node.has_children()) {
				if (0 == ray.sign[node.axis()]) {
					node_stack.push(node.end_index);
					n = n + 1;
				} else {
					node_stack.push(n + 1);
					n = node.end_index;
				}
			} else {
				for (uint32_t i = node.start_index; i < node.end_index; ++i) {
                    if (data_.intersect(i, ray, uv)) {
                        uv = data_.interpolate_uv(i, uv);
                        opacity += (1.f - opacity) * materials[data_.material_index(i)]->opacity(uv, sampler);
						if (opacity >= 1.f) {
							return 1.f;
						}
						ray.max_t = max_t;
					}
				}

				n = node_stack.pop();
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
void Tree<Data>::allocate_triangles(uint32_t num_triangles) {
    data_.allocate_triangles(num_triangles);
	num_parts_ = 0;
}

template<typename Data>
void Tree<Data>::add_triangle(const Vertex& a, const Vertex& b, const Vertex& c, uint32_t material_index) {
    data_.add_triangle(a, b, c, material_index);
	num_parts_ = std::max(num_parts_, material_index + 1);
}

}}}}
