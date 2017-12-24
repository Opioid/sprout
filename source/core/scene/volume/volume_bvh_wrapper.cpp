#include "volume_bvh_wrapper.hpp"
#include "volume.hpp"
#include "scene/scene_ray.inl"
#include "scene/scene_worker.hpp"
#include "scene/bvh/scene_bvh_node.inl"
#include "scene/bvh/scene_bvh_tree.inl"
#include "scene/shape/node_stack.inl"

namespace scene::volume {

bvh::Tree<Volume>& BVH_wrapper::tree() {
	return tree_;
}

void BVH_wrapper::set_infinite_props(const std::vector<Volume*>& infite_props) {
	num_infinite_props_ = static_cast<uint32_t>(infite_props.size());
	infinite_props_	 = infite_props.data();
}

const math::AABB& BVH_wrapper::aabb() const {
	return tree_.aabb_;
}

const Volume* BVH_wrapper::intersect(scene::Ray& ray, shape::Node_stack& node_stack,
									 bool include_infinite, float& epsilon, bool& inside) const {
	const Volume* volume = nullptr;
	inside = false;

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

	bvh::Node*     nodes   = tree_.nodes_;
	Volume* const* volumes = tree_.data_.data();

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
				const auto v = volumes[i];
				if (v->intersect(ray, node_stack, epsilon, inside)) {
					volume = v;
					ray_max_t = simd::load_float(&ray.max_t);
				}
			}
		}

		n = node_stack.pop();
	}

	if (!inside && include_infinite) {
		for (uint32_t i = 0, len = num_infinite_props_; i < len; ++i) {
			const auto v = infinite_props_[i];
			if (v->intersect(ray, node_stack, epsilon, inside)) {
				volume = v;
			}
		}
	}

	return volume;
}

}
