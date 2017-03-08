#pragma once

#include "triangle_bvh_tree.hpp"
#include "triangle_bvh_node.inl"
#include "scene/scene_worker.hpp"
#include "scene/shape/node_stack.inl"
#include "scene/shape/triangle/triangle_intersection.hpp"
#include "base/math/aabb.inl"
#include "base/math/vector3.inl"
#include "base/memory/align.inl"

namespace scene { namespace shape { namespace triangle { namespace bvh {

template<typename Data>
Tree<Data>::Tree() : num_nodes_(0), nodes_(nullptr), num_parts_(0), num_part_triangles_(nullptr) {}

template<typename Data>
Tree<Data>::~Tree() {
	delete[] num_part_triangles_;

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
math::AABB Tree<Data>::aabb() const {
	if (nodes_) {
		return math::AABB(float3(nodes_[0].min()), float3(nodes_[0].max()));
	} else {
		return math::AABB::empty();
	}
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
bool Tree<Data>::intersect(math::Ray& ray, Node_stack& node_stack,
						   Intersection& intersection) const {
	node_stack.clear();
	node_stack.push(0);
	uint32_t n = 0;

	float2 uv;
	uint32_t index = 0xFFFFFFFF;

	math::simd::Vector ray_origin		 = math::simd::load_float3(ray.origin);
	math::simd::Vector ray_inv_direction = math::simd::load_float3(ray.inv_direction);
	math::simd::Vector ray_min_t = _mm_set1_ps(ray.min_t);
	math::simd::Vector ray_max_t = _mm_set1_ps(ray.max_t);

	while (!node_stack.empty()) {
		const auto& node = nodes_[n];

		if (node.intersect_p(ray_origin, ray_inv_direction, ray_min_t, ray_max_t)) {
			if (0 == node.num_primitives()) {
				const uint32_t nn[]{node.next(), n + 1};
				const uint8_t s = ray.signs[node.axis()];
				node_stack.push(nn[s]);
				n = nn[s ^ 0x01];
				continue;
			}

			for (uint32_t i = node.indices_start(), len = node.indices_end(); i < len; ++i) {
				if (data_.intersect(i, ray, uv)) {
					index = i;
					// ray.max_t has changed if intersect() returns true!
					ray_max_t = _mm_set1_ps(ray.max_t);
				}
			}
		}

		n = node_stack.pop();
	}

	intersection.uv = uv;
	intersection.index = index;

	return index != 0xFFFFFFFF;
}

template<typename Data>
bool Tree<Data>::intersect_p(const math::Ray& ray, Node_stack& node_stack) const {
	node_stack.clear();
	node_stack.push(0);
	uint32_t n = 0;

	math::simd::Vector ray_origin		 = math::simd::load_float3(ray.origin);
//	math::simd::Vector ray_direction = math::simd::load_float3(ray.direction);
	math::simd::Vector ray_inv_direction = math::simd::load_float3(ray.inv_direction);
	math::simd::Vector ray_min_t = _mm_set1_ps(ray.min_t);
	math::simd::Vector ray_max_t = _mm_set1_ps(ray.max_t);

	while (!node_stack.empty()) {
		const auto& node = nodes_[n];

		if (node.intersect_p(ray_origin, ray_inv_direction, ray_min_t, ray_max_t)) {
			if (0 == node.num_primitives()) {
				const uint32_t nn[]{node.next(), n + 1};
				const uint8_t s = ray.signs[node.axis()];
				node_stack.push(nn[s]);
				n = nn[s ^ 0x01];
				continue;
			}

			for (uint32_t i = node.indices_start(), len = node.indices_end(); i < len; ++i) {
				if (data_.intersect_p(i, ray)) {
			//	if (data_.intersect_p(ray_origin, ray_direction, ray_min_t, ray_max_t, i)) {
					return true;
				}
			}
		}

		n = node_stack.pop();
	}

	return false;
}

template<typename Data>
float Tree<Data>::opacity(math::Ray& ray, float time, const material::Materials& materials,
						  Worker& worker, material::Sampler_settings::Filter filter) const {
	auto& node_stack = worker.node_stack();
	node_stack.clear();
	node_stack.push(0);
	uint32_t n = 0;

	float opacity = 0.f;

	float2 uv;
	float max_t = ray.max_t;

	math::simd::Vector ray_origin		 = math::simd::load_float3(ray.origin);
	math::simd::Vector ray_inv_direction = math::simd::load_float3(ray.inv_direction);
	math::simd::Vector ray_min_t = _mm_set1_ps(ray.min_t);
	math::simd::Vector ray_max_t = _mm_set1_ps(ray.max_t);

	while (!node_stack.empty()) {
		auto& node = nodes_[n];

		if (node.intersect_p(ray_origin, ray_inv_direction, ray_min_t, ray_max_t)) {
			if (0 == node.num_primitives()) {
				const uint32_t nn[]{node.next(), n + 1};
				const uint8_t s = ray.signs[node.axis()];
				node_stack.push(nn[s]);
				n = nn[s ^ 0x01];
				continue;
			}

			for (uint32_t i = node.indices_start(), len = node.indices_end(); i < len; ++i) {
				if (data_.intersect(i, ray, uv)) {
					uv = data_.interpolate_uv(i, uv);

					const auto material = materials[data_.material_index(i)];

					opacity += (1.f - opacity) * material->opacity(uv, time, worker, filter);
					if (opacity >= 1.f) {
						return 1.f;
					}

					ray.max_t = max_t;
					// ray.max_t has changed if intersect() returns true!
					// ray_max_t = _mm_set1_ps(max_t);
				}
			}
		}

		n = node_stack.pop();
	}

	return opacity;
}

template<typename Data>
float3 Tree<Data>::absorption(math::Ray& ray, float time, const material::Materials& materials,
							  Worker& worker, material::Sampler_settings::Filter filter) const {
	auto& node_stack = worker.node_stack();
	node_stack.clear();
	node_stack.push(0);
	uint32_t n = 0;

	float3 absorption(0.f);

	float2 uv;
	float max_t = ray.max_t;

	math::simd::Vector ray_origin		 = math::simd::load_float3(ray.origin);
	math::simd::Vector ray_inv_direction = math::simd::load_float3(ray.inv_direction);
	math::simd::Vector ray_min_t = _mm_set1_ps(ray.min_t);
	math::simd::Vector ray_max_t = _mm_set1_ps(ray.max_t);

	while (!node_stack.empty()) {
		auto& node = nodes_[n];

		if (node.intersect_p(ray_origin, ray_inv_direction, ray_min_t, ray_max_t)) {
			if (0 == node.num_primitives()) {
				const uint32_t nn[]{node.next(), n + 1};
				const uint8_t s = ray.signs[node.axis()];
				node_stack.push(nn[s]);
				n = nn[s ^ 0x01];
				continue;
			}

			for (uint32_t i = node.indices_start(), len = node.indices_end(); i < len; ++i) {
				if (data_.intersect(i, ray, uv)) {
					uv = data_.interpolate_uv(i, uv);

					const float3 normal = data_.normal(i);

					const auto material = materials[data_.material_index(i)];

					const float3 ta = material->thin_absorption(ray.direction, normal, uv,
																	time, worker, filter);
					absorption += (1.f - absorption) * ta;
					if (math::all_greater_equal(absorption, 1.f)) {
						return float3(1.f);
					}

					ray.max_t = max_t;
					// ray.max_t has changed if intersect() returns true!
					// ray_max_t = _mm_set1_ps(max_t);
				}
			}
		}

		n = node_stack.pop();
	}

	return absorption;
}

template<typename Data>
void Tree<Data>::interpolate_triangle_data(uint32_t index, float2 uv,
										   float3& n, float3& t, float2& tc) const {
	data_.interpolate_data(index, uv, n, t, tc);
}

template<typename Data>
float2 Tree<Data>::interpolate_triangle_uv(uint32_t index, float2 uv) const {
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
float3 Tree<Data>::triangle_normal(uint32_t index) const {
    return data_.normal(index);
}

template<typename Data>
float Tree<Data>::triangle_area(uint32_t index) const {
	return data_.area(index);
}

template<typename Data>
float Tree<Data>::triangle_area(uint32_t index, float3_p scale) const {
    return data_.area(index, scale);
}

template<typename Data>
void Tree<Data>::sample(uint32_t index, float2 r2, float3& p,
						float3& n, float2& tc) const {
    data_.sample(index, r2, p, n, tc);
}

template<typename Data>
void Tree<Data>::sample(uint32_t index, float2 r2, float3& p, float2& tc) const {
	data_.sample(index, r2, p, tc);
}

template<typename Data>
void Tree<Data>::sample(uint32_t index, float2 r2, float3& p) const {
	data_.sample(index, r2, p);
}

template<typename Data>
void Tree<Data>::allocate_parts(uint32_t num_parts) {
	if (num_parts != num_parts_) {
		num_parts_ = num_parts;
		delete[] num_part_triangles_;
		num_part_triangles_ = new uint32_t[num_parts];
	}

	for (uint32_t i = 0; i < num_parts; ++i) {
		num_part_triangles_[i] = 0;
	}
}

template<typename Data>
void Tree<Data>::allocate_triangles(uint32_t num_triangles, const std::vector<Vertex>& vertices) {
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
	return sizeof(*this) +
		   num_nodes_ * sizeof(Node) +
		   num_parts_ * sizeof(uint32_t) +
		   data_.num_bytes();
}

}}}}
