#include "scene_bvh_tree.hpp"
#include "scene_bvh_node.inl"
#include "scene/prop.hpp"
#include "scene/scene_ray.inl"
#include "scene/scene_worker.hpp"
#include "scene/shape/node_stack.inl"
#include "base/math/aabb.inl"
#include "base/math/vector3.inl"
#include "base/memory/align.hpp"

namespace scene { namespace bvh {

Tree::~Tree() {
	memory::free_aligned(nodes_);
}

void Tree::clear() {
	infinite_props_start_ = 0;
	infinite_props_end_ = 0;

	props_.clear();
}

bvh::Node* Tree::allocate_nodes(uint32_t num_nodes) {
	if (num_nodes != num_nodes_) {
		num_nodes_ = num_nodes;

		memory::free_aligned(nodes_);
		nodes_ = memory::allocate_aligned<Node>(num_nodes);
	}

	return nodes_;
}

const math::AABB& Tree::aabb() const {
	return aabb_;
}

bool Tree::intersect(scene::Ray& ray, shape::Node_stack& node_stack,
					 Intersection& intersection) const {
	bool hit = false;
	const Prop* prop = nullptr;

	node_stack.clear();
	if (0 != num_nodes_) {
		node_stack.push(0);
	}

	uint32_t n = 0;

	const Vector ray_origin		   = simd::load_float4(ray.origin.v);
//	const Vector ray_direction	   = simd::load_float4(ray.direction.v);
	const Vector ray_inv_direction = simd::load_float4(ray.inv_direction.v);
	const Vector ray_min_t		   = simd::load_float(&ray.min_t);
		  Vector ray_max_t		   = simd::load_float(&ray.max_t);

	while (!node_stack.empty()) {
		const auto& node = nodes_[n];

		if (node.intersect_p(ray_origin, ray_inv_direction, ray_min_t, ray_max_t)) {
			if (0 == node.num_primitives()) {
				if (0 == ray.signs[node.axis()]) {
					node_stack.push(node.next());
					++n;
				} else {
					node_stack.push(n + 1);
					n = node.next();
				}

				continue;
			}

			for (uint32_t i = node.indices_start(), len = node.indices_end(); i < len; ++i) {
				const auto p = props_[i];
				if (p->intersect(ray, node_stack, intersection.geo)) {
					prop = p;
					hit = true;
				}
			}
		}

		n = node_stack.pop();
	}

	for (uint32_t i = infinite_props_start_; i < infinite_props_end_; ++i) {
		const auto p = props_[i];
		if (p->intersect(ray, node_stack, intersection.geo)) {
			prop = p;
			hit = true;
		}
	}

	intersection.prop = prop;
	return hit;
}

bool Tree::intersect_p(const scene::Ray& ray, shape::Node_stack& node_stack) const {
	node_stack.clear();
	if (0 != num_nodes_) {
		node_stack.push(0);
	}

	uint32_t n = 0;

	const Vector ray_origin		   = simd::load_float4(ray.origin.v);
//	const Vector ray_direction	   = simd::load_float4(ray.direction.v);
	const Vector ray_inv_direction = simd::load_float4(ray.inv_direction.v);
	const Vector ray_min_t		   = simd::load_float(&ray.min_t);
		  Vector ray_max_t		   = simd::load_float(&ray.max_t);

	while (!node_stack.empty()) {
		const auto& node = nodes_[n];

		if (node.intersect_p(ray_origin, ray_inv_direction, ray_min_t, ray_max_t)) {
			if (0 == node.num_primitives()) {
				if (0 == ray.signs[node.axis()]) {
					node_stack.push(node.next());
					++n;
				} else {
					node_stack.push(n + 1);
					n = node.next();
				}

				continue;
			}

			for (uint32_t i = node.indices_start(), len = node.indices_end(); i < len; ++i) {
				if (props_[i]->intersect_p(ray, node_stack)) {
//				if (props_[i]->intersect_p(ray_origin, ray_direction, ray_inv_direction,
//										   ray_min_t, ray_max_t, ray.time, node_stack)) {
					return true;
				}
			}
		}

		n = node_stack.pop();
	}

	for (uint32_t i = infinite_props_start_; i < infinite_props_end_; ++i) {
		if (props_[i]->intersect_p(ray, node_stack)) {
			return true;
		}
	}

	return false;
}

float Tree::opacity(const scene::Ray& ray, Sampler_filter filter, const Worker& worker) const {
	auto& node_stack = worker.node_stack();

	node_stack.clear();
	if (0 != num_nodes_) {
		node_stack.push(0);
	}

	uint32_t n = 0;

	float opacity = 0.f;

	const Vector ray_origin		   = simd::load_float4(ray.origin.v);
//	const Vector ray_direction	   = simd::load_float4(ray.direction.v);
	const Vector ray_inv_direction = simd::load_float4(ray.inv_direction.v);
	const Vector ray_min_t		   = simd::load_float(&ray.min_t);
		  Vector ray_max_t		   = simd::load_float(&ray.max_t);

	while (!node_stack.empty()) {
		auto& node = nodes_[n];

		if (node.intersect_p(ray_origin, ray_inv_direction, ray_min_t, ray_max_t)) {
			if (0 == node.num_primitives()) {
				if (0 == ray.signs[node.axis()]) {
					node_stack.push(node.next());
					++n;
				} else {
					node_stack.push(n + 1);
					n = node.next();
				}

				continue;
			}

			for (uint32_t i = node.indices_start(), len = node.indices_end(); i < len; ++i) {
				const auto p = props_[i];
				opacity += (1.f - opacity) * p->opacity(ray, filter, worker);

				if (opacity >= 1.f) {
					return 1.f;
				}
			}
		}

		n = node_stack.pop();
	}

	for (uint32_t i = infinite_props_start_; i < infinite_props_end_; ++i) {
		const auto p = props_[i];
		opacity += (1.f - opacity) * p->opacity(ray, filter, worker);
		if (opacity >= 1.f) {
			return 1.f;
		}
	}

	return opacity;
}

float3 Tree::thin_absorption(const scene::Ray& ray, Sampler_filter filter,
							 const Worker& worker) const {
	auto& node_stack = worker.node_stack();

	node_stack.clear();
	if (0 != num_nodes_) {
		node_stack.push(0);
	}

	uint32_t n = 0;

	float3 absorption(0.f);

	const Vector ray_origin		   = simd::load_float4(ray.origin.v);
//	const Vector ray_direction	   = simd::load_float4(ray.direction.v);
	const Vector ray_inv_direction = simd::load_float4(ray.inv_direction.v);
	const Vector ray_min_t		   = simd::load_float(&ray.min_t);
		  Vector ray_max_t		   = simd::load_float(&ray.max_t);

	while (!node_stack.empty()) {
		auto& node = nodes_[n];

		if (node.intersect_p(ray_origin, ray_inv_direction, ray_min_t, ray_max_t)) {
			if (0 == node.num_primitives()) {
				if (0 == ray.signs[node.axis()]) {
					node_stack.push(node.next());
					++n;
				} else {
					node_stack.push(n + 1);
					n = node.next();
				}

				continue;
			}

			for (uint32_t i = node.indices_start(), len = node.indices_end(); i < len; ++i) {
				const auto p = props_[i];
				absorption += (1.f - absorption) * p->thin_absorption(ray, filter, worker);
				if (math::all_greater_equal(absorption, 1.f)) {
					return float3(1.f);
				}
			}
		}

		n = node_stack.pop();
	}

	for (uint32_t i = infinite_props_start_; i < infinite_props_end_; ++i) {
		const auto p = props_[i];
		absorption += (1.f - absorption) * p->thin_absorption(ray, filter, worker);
		if (math::all_greater_equal(absorption, 1.f)) {
			return float3(1.f);
		}
	}

	return absorption;
}

}}
