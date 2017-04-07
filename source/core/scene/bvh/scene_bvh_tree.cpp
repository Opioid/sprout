#include "scene_bvh_tree.hpp"
#include "scene_bvh_node.inl"
#include "scene/prop.hpp"
#include "scene/scene_ray.inl"
#include "scene/scene_worker.hpp"
#include "base/math/aabb.inl"
#include "base/math/vector3.inl"
#include "base/memory/align.hpp"

namespace scene { namespace bvh {

Build_node::Build_node() {
	children[0] = nullptr;
	children[1] = nullptr;
}

Build_node::~Build_node() {
	delete children[0];
	delete children[1];
}

bool Build_node::intersect(scene::Ray& ray, const std::vector<Prop*>& props,
						   shape::Node_stack& node_stack, Intersection& intersection) const {
	if (!aabb.intersect_p(ray)) {
		return false;
	}

	bool hit = false;

	if (children[0]) {
		const uint8_t c = ray.signs[axis];

		if (children[c]->intersect(ray, props, node_stack, intersection)) {
			hit = true;
		}

		if (children[1 - c]->intersect(ray, props, node_stack, intersection)) {
			hit = true;
		}
	} else {
		for (uint32_t i = offset; i < props_end; ++i) {
			const auto p = props[i];
			if (p->intersect(ray, node_stack, intersection.geo)) {
				intersection.prop = p;
				hit = true;
			}
		}
	}

	return hit;
}

bool Build_node::intersect_p(const scene::Ray& ray, const std::vector<Prop*>& props,
							 shape::Node_stack& node_stack) const {
	if (!aabb.intersect_p(ray)) {
		return false;
	}

	if (children[0]) {
		const uint8_t c = ray.signs[axis];

		if (children[c]->intersect_p(ray, props, node_stack)) {
			return true;
		}

		return children[1 - c]->intersect_p(ray, props, node_stack);
	}

	for (uint32_t i = offset; i < props_end; ++i) {
		if (props[i]->intersect_p(ray, node_stack)) {
			return true;
		}
	}

	return false;
}

float Build_node::opacity(const scene::Ray& ray, const std::vector<Prop*>& props,
						  Worker& worker, Sampler_filter filter) const {
	if (!aabb.intersect_p(ray)) {
		return 0.f;
	}

	float opacity = 0.f;

	if (children[0]) {
		const uint8_t c = ray.signs[axis];

		opacity += (1.f - opacity) * children[c]->opacity(ray, props, worker, filter);
		if (opacity >= 1.f) {
			return 1.f;
		}

		opacity += (1.f - opacity) * children[1 - c]->opacity(ray, props, worker, filter);
		if (opacity >= 1.f) {
			return 1.f;
		}
	} else {
		for (uint32_t i = offset; i < props_end; ++i) {
			auto p = props[i];
			opacity += (1.f - opacity) * p->opacity(ray, worker, filter);
			if (opacity >= 1.f) {
				return 1.f;
			}
		}
	}

	return opacity;
}

float3 Build_node::thin_absorption(const scene::Ray& ray, const std::vector<Prop*>& props,
								   Worker& worker, Sampler_filter filter) const {
	if (!aabb.intersect_p(ray)) {
		return float3(0.f);
	}

	float3 absorption(0.f);

	if (children[0]) {
		const uint8_t c = ray.signs[axis];

		float3 ta = children[c]->thin_absorption(ray, props, worker, filter);
		absorption += (1.f - absorption) * ta;
		if (math::all_greater_equal(absorption, 1.f)) {
			return float3(1.f);
		}

		ta = children[1 - c]->thin_absorption(ray, props, worker, filter);
		absorption += (1.f - absorption) * ta;
		if (math::all_greater_equal(absorption, 1.f)) {
			return float3(1.f);
		}
	} else {
		for (uint32_t i = offset; i < props_end; ++i) {
			const auto p = props[i];
			absorption += (1.f - absorption) * p->thin_absorption(ray, worker, filter);
			if (math::all_greater_equal(absorption, 1.f)) {
				return float3(1.f);
			}
		}
	}

	return absorption;
}

Tree::Tree() : num_nodes_(0), nodes_(nullptr) {}

Tree::~Tree() {
	memory::free_aligned(nodes_);
}

void Tree::clear() {
	delete root_.children[0];
	root_.children[0] = nullptr;

	delete root_.children[1];
	root_.children[1] = nullptr;

	root_.offset = 0;
	root_.props_end = 0;

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
	return root_.aabb;
}

bool Tree::intersect(scene::Ray& ray, shape::Node_stack& node_stack,
					 Intersection& intersection) const {
	bool hit = false;
	const Prop* prop = nullptr;

	node_stack.clear();
	node_stack.push(0);
	uint32_t n = 0;

	const Vector ray_origin		   = simd::load_float4(ray.origin);
//	const Vector ray_direction	   = simd::load_float4(ray.direction);
	const Vector ray_inv_direction = simd::load_float4(ray.inv_direction);
	const Vector ray_min_t		   = simd::load_float(ray.min_t);
		  Vector ray_max_t		   = simd::load_float(ray.max_t);

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
	node_stack.push(0);
	uint32_t n = 0;

	const Vector ray_origin		   = simd::load_float4(ray.origin);
//	const Vector ray_direction	   = simd::load_float4(ray.direction);
	const Vector ray_inv_direction = simd::load_float4(ray.inv_direction);
	const Vector ray_min_t		   = simd::load_float(ray.min_t);
		  Vector ray_max_t		   = simd::load_float(ray.max_t);

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

float Tree::opacity(const scene::Ray& ray, Worker& worker,
					material::Sampler_settings::Filter filter) const {
	auto& node_stack = worker.node_stack();
	node_stack.clear();
	node_stack.push(0);
	uint32_t n = 0;

	float opacity = 0.f;

	const Vector ray_origin		   = simd::load_float4(ray.origin);
//	const Vector ray_direction	   = simd::load_float4(ray.direction);
	const Vector ray_inv_direction = simd::load_float4(ray.inv_direction);
	const Vector ray_min_t		   = simd::load_float(ray.min_t);
		  Vector ray_max_t		   = simd::load_float(ray.max_t);

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
				opacity += (1.f - opacity) * p->opacity(ray, worker, filter);

				if (opacity >= 1.f) {
					return 1.f;
				}
			}
		}

		n = node_stack.pop();
	}

	for (uint32_t i = infinite_props_start_; i < infinite_props_end_; ++i) {
		const auto p = props_[i];
		opacity += (1.f - opacity) * p->opacity(ray, worker, filter);
		if (opacity >= 1.f) {
			return 1.f;
		}
	}

	return opacity;
}

float3 Tree::thin_absorption(const scene::Ray& ray, Worker& worker,
							 material::Sampler_settings::Filter filter) const {
	auto& node_stack = worker.node_stack();
	node_stack.clear();
	node_stack.push(0);
	uint32_t n = 0;

	float3 absorption(0.f);

	const Vector ray_origin		   = simd::load_float3(ray.origin);
//	const Vector ray_direction	   = simd::load_float3(ray.direction);
	const Vector ray_inv_direction = simd::load_float3(ray.inv_direction);
	const Vector ray_min_t		   = simd::load_float(ray.min_t);
		  Vector ray_max_t		   = simd::load_float(ray.max_t);

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
				absorption += (1.f - absorption) * p->thin_absorption(ray, worker, filter);
				if (math::all_greater_equal(absorption, 1.f)) {
					return float3(1.f);
				}
			}
		}

		n = node_stack.pop();
	}

	for (uint32_t i = infinite_props_start_; i < infinite_props_end_; ++i) {
		const auto p = props_[i];
		absorption += (1.f - absorption) * p->thin_absorption(ray, worker, filter);
		if (math::all_greater_equal(absorption, 1.f)) {
			return float3(1.f);
		}
	}

	return absorption;
}

}}
