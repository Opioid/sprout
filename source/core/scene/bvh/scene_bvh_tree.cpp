#include "scene_bvh_tree.hpp"
#include "scene/scene_ray.inl"
#include "scene/prop.hpp"
#include "base/math/aabb.inl"
#include "base/math/vector3.inl"

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
		const int8_t c = ray.signs[axis];

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

const math::aabb& Tree::aabb() const {
	return root_.aabb;
}

bool Tree::intersect(scene::Ray& ray, shape::Node_stack& node_stack,
					 Intersection& intersection) const {
	bool hit = false;
	intersection.prop = nullptr;

	if (root_.intersect(ray, props_, node_stack, intersection)) {
		hit = true;
	}

	const Prop* prop = intersection.prop;

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
	if (root_.intersect_p(ray, props_, node_stack)) {
		return true;
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
	float opacity = root_.opacity(ray, props_, worker, filter);

	if (opacity < 1.f) {
		for (uint32_t i = infinite_props_start_; i < infinite_props_end_; ++i) {
			auto p = props_[i];
			opacity += (1.f - opacity) * p->opacity(ray, worker, filter);
			if (opacity >= 1.f) {
				return 1.f;
			}
		}
	}

	return opacity;
}

float3 Tree::thin_absorption(const scene::Ray& ray, Worker& worker,
							 material::Sampler_settings::Filter filter) const {
	float3 absorption = root_.thin_absorption(ray, props_, worker, filter);

	if (math::all_lesser(absorption, 1.f)) {
		for (uint32_t i = infinite_props_start_; i < infinite_props_end_; ++i) {
			auto p = props_[i];
			absorption += (1.f - absorption) * p->thin_absorption(ray, worker, filter);
			if (math::all_greater_equal(absorption, 1.f)) {
				return float3(1.f);
			}
		}
	}

	return absorption;
}

}}
