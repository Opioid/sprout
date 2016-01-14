#include "scene_bvh_tree.hpp"
#include "scene/prop/prop.hpp"
#include "base/math/bounding/aabb.inl"
#include <iostream>

namespace scene { namespace bvh {

Build_node::Build_node() {
	children[0] = nullptr;
	children[1] = nullptr;
}

Build_node::~Build_node() {
	delete children[0];
	delete children[1];
}

bool Build_node::intersect(math::Oray& ray, const std::vector<Prop*>& props, shape::Node_stack& node_stack,
						   Intersection& intersection) const {
	if (!aabb.intersect_p(ray)) {
		return false;
	}

	bool hit = false;

	if (children[0]) {
		int8_t c = ray.sign[axis];

		if (children[c]->intersect(ray, props, node_stack, intersection)) {
			hit = true;
		}

		if (children[1 - c]->intersect(ray, props, node_stack, intersection)) {
			hit = true;
		}
	} else {
		for (uint32_t i = offset; i < props_end; ++i) {
			auto p = props[i];
			if (p->intersect(ray, node_stack, intersection.geo)) {
				intersection.prop = p;
				hit = true;
			}
		}
	}

	return hit;
}

bool Build_node::intersect_p(const math::Oray& ray, const std::vector<Prop*>& props,
							 shape::Node_stack& node_stack) const {
	if (!aabb.intersect_p(ray)) {
		return false;
	}

	if (children[0]) {
		uint8_t c = ray.sign[axis];

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

float Build_node::opacity(const math::Oray& ray, const std::vector<Prop*>& props, shape::Node_stack& node_stack,
						  const image::texture::sampler::Sampler_2D& sampler) const {
	if (!aabb.intersect_p(ray)) {
		return 0.f;
	}

	float opacity = 0.f;

	if (children[0]) {
		uint8_t c = ray.sign[axis];

		opacity += (1.f - opacity) * children[c]->opacity(ray, props, node_stack, sampler);
		if (opacity >= 1.f) {
			return 1.f;
		}

		opacity += (1.f - opacity) * children[1 - c]->opacity(ray, props, node_stack, sampler);
		if (opacity >= 1.f) {
			return 1.f;
		}
	} else {
		for (uint32_t i = offset; i < props_end; ++i) {
			auto p = props[i];
			opacity += (1.f - opacity) * p->opacity(ray, node_stack, sampler);
			if (opacity >= 1.f) {
				return 1.f;
			}
		}
	}

	return opacity;
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

bool Tree::intersect(math::Oray& ray, shape::Node_stack& node_stack, Intersection& intersection) const {
	bool hit = false;

	if (root_.intersect(ray, props_, node_stack, intersection)) {
		hit = true;
	}

	for (uint32_t i = infinite_props_start_; i < infinite_props_end_; ++i) {
		auto p = props_[i];
		if (p->intersect(ray, node_stack, intersection.geo)) {
			intersection.prop = p;
			hit = true;
		}
	}

	return hit;
}

bool Tree::intersect_p(const math::Oray& ray, shape::Node_stack& node_stack) const {
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

float Tree::opacity(const math::Oray& ray, shape::Node_stack& node_stack,
					const image::texture::sampler::Sampler_2D& sampler) const {
	float opacity = root_.opacity(ray, props_, node_stack, sampler);

	if (opacity < 1.f) {
		for (uint32_t i = infinite_props_start_; i < infinite_props_end_; ++i) {
			auto p = props_[i];
			opacity += p->opacity(ray, node_stack, sampler);
			if (opacity > 1.f) {
				return 1.f;
			}
		}
	}

	return opacity;
}

}}
