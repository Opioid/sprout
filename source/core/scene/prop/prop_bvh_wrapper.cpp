#include "prop_bvh_wrapper.hpp"
#include "prop.hpp"
#include "prop_intersection.hpp"
#include "scene/scene_ray.inl"
#include "scene/scene_worker.hpp"
#include "scene/bvh/scene_bvh_node.inl"
#include "scene/bvh/scene_bvh_tree.inl"
#include "scene/shape/node_stack.inl"

namespace scene::prop {

bvh::Tree<Prop>& BVH_wrapper::tree() {
	return tree_;
}

void BVH_wrapper::set_infinite_props(const std::vector<Prop*>& infite_props) {
	num_infinite_props_ = static_cast<uint32_t>(infite_props.size());
	infinite_props_	 = infite_props.data();
}

const math::AABB& BVH_wrapper::aabb() const {
	return tree_.aabb_;
}

bool BVH_wrapper::intersect(scene::Ray& ray, shape::Node_stack& node_stack,
							Intersection& intersection) const {
	bool hit = false;
	const Prop* prop = nullptr;

	node_stack.clear();
	if (0 != tree_.num_nodes_) {
		node_stack.push(0);
	}

	uint32_t n = 0;

	const Vector ray_origin		   = simd::load_float4(ray.origin.v);
//	const Vector ray_direction	   = simd::load_float4(ray.direction.v);
	const Vector ray_inv_direction = simd::load_float4(ray.inv_direction.v);
	const Vector ray_min_t		   = simd::load_float(&ray.min_t);
		  Vector ray_max_t		   = simd::load_float(&ray.max_t);

	bvh::Node*   nodes = tree_.nodes_;
	Prop* const* props = tree_.data_.data();

	while (!node_stack.empty()) {
		const auto& node = nodes[n];

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
				const auto p = props[i];
				if (p->intersect(ray, node_stack, intersection.geo)) {
					prop = p;
					hit = true;
					ray_max_t = simd::load_float(&ray.max_t);
				}
			}
		}

		n = node_stack.pop();
	}

	for (uint32_t i = 0, len = num_infinite_props_; i < len; ++i) {
		const auto p = infinite_props_[i];
		if (p->intersect(ray, node_stack, intersection.geo)) {
			prop = p;
			hit = true;
		}
	}

	intersection.prop = prop;
	intersection.geo.subsurface = false;
	return hit;
}

bool BVH_wrapper::intersect_p(const scene::Ray& ray, shape::Node_stack& node_stack) const {
	node_stack.clear();
	if (0 != tree_.num_nodes_) {
		node_stack.push(0);
	}

	uint32_t n = 0;

	const Vector ray_origin		   = simd::load_float4(ray.origin.v);
//	const Vector ray_direction	   = simd::load_float4(ray.direction.v);
	const Vector ray_inv_direction = simd::load_float4(ray.inv_direction.v);
	const Vector ray_min_t		   = simd::load_float(&ray.min_t);
		  Vector ray_max_t		   = simd::load_float(&ray.max_t);

	bvh::Node*   nodes = tree_.nodes_;
	Prop* const* props = tree_.data_.data();

	while (!node_stack.empty()) {
		const auto& node = nodes[n];

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
				if (props[i]->intersect_p(ray, node_stack)) {
//				if (props[i]->intersect_p(ray_origin, ray_direction, ray_inv_direction,
//										  ray_min_t, ray_max_t, ray.time, node_stack)) {
					return true;
				}
			}
		}

		n = node_stack.pop();
	}

	for (uint32_t i = 0, len = num_infinite_props_; i < len; ++i) {
		if (infinite_props_[i]->intersect_p(ray, node_stack)) {
			return true;
		}
	}

	return false;
}

float BVH_wrapper::opacity(const scene::Ray& ray, Sampler_filter filter,
						   const Worker& worker) const {
	auto& node_stack = worker.node_stack();

	node_stack.clear();
	if (0 != tree_.num_nodes_) {
		node_stack.push(0);
	}

	uint32_t n = 0;

	float opacity = 0.f;

	const Vector ray_origin		   = simd::load_float4(ray.origin.v);
//	const Vector ray_direction	   = simd::load_float4(ray.direction.v);
	const Vector ray_inv_direction = simd::load_float4(ray.inv_direction.v);
	const Vector ray_min_t		   = simd::load_float(&ray.min_t);
		  Vector ray_max_t		   = simd::load_float(&ray.max_t);

	bvh::Node*   nodes = tree_.nodes_;
	Prop* const* props = tree_.data_.data();

	while (!node_stack.empty()) {
		auto& node = nodes[n];

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
				const auto p = props[i];
				opacity += (1.f - opacity) * p->opacity(ray, filter, worker);

				if (opacity >= 1.f) {
					return 1.f;
				}
			}
		}

		n = node_stack.pop();
	}

	for (uint32_t i = 0, len = num_infinite_props_; i < len; ++i) {
		const auto p = infinite_props_[i];
		opacity += (1.f - opacity) * p->opacity(ray, filter, worker);
		if (opacity >= 1.f) {
			return 1.f;
		}
	}

	return opacity;
}

float3 BVH_wrapper::thin_absorption(const scene::Ray& ray, Sampler_filter filter,
									const Worker& worker) const {
	auto& node_stack = worker.node_stack();

	node_stack.clear();
	if (0 != tree_.num_nodes_) {
		node_stack.push(0);
	}

	uint32_t n = 0;

	float3 absorption(0.f);

	const Vector ray_origin		   = simd::load_float4(ray.origin.v);
//	const Vector ray_direction	   = simd::load_float4(ray.direction.v);
	const Vector ray_inv_direction = simd::load_float4(ray.inv_direction.v);
	const Vector ray_min_t		   = simd::load_float(&ray.min_t);
		  Vector ray_max_t		   = simd::load_float(&ray.max_t);

	bvh::Node*   nodes = tree_.nodes_;
	Prop* const* props = tree_.data_.data();

	while (!node_stack.empty()) {
		auto& node = nodes[n];

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
				const auto p = props[i];
				absorption += (1.f - absorption) * p->thin_absorption(ray, filter, worker);
				if (math::all_greater_equal(absorption, 1.f)) {
					return float3(1.f);
				}
			}
		}

		n = node_stack.pop();
	}

	for (uint32_t i = 0, len = num_infinite_props_; i < len; ++i) {
		const auto p = infinite_props_[i];
		absorption += (1.f - absorption) * p->thin_absorption(ray, filter, worker);
		if (math::all_greater_equal(absorption, 1.f)) {
			return float3(1.f);
		}
	}

	return absorption;
}

}
