#include "scene_bvh_builder.hpp"
#include "scene/prop/prop.hpp"
#include "scene/shape/shape.hpp"
#include "base/math/plane.inl"
#include "base/math/bounding/aabb.inl"
#include <iostream>

namespace scene { namespace bvh {

void Builder::build(Tree& tree, const std::vector<Prop*>& props) {
	tree.props_.clear();
	tree.props_.reserve(props.size());

	split(&tree.root_, props, 4, tree.props_);

	tree.infinite_props_start_ = static_cast<uint32_t>(tree.props_.size());

	for (auto p : props) {
		if (!p->shape()->is_finite()) {
			tree.props_.push_back(p);
		}
	}

	tree.infinite_props_end_ = static_cast<uint32_t>(tree.props_.size());
}

void Builder::split(Build_node* node, const std::vector<Prop*>& props, size_t max_shapes, std::vector<Prop*>& out_props) {
	node->aabb = aabb(props);

	if (props.size() <= max_shapes) {
		assign(node, props, out_props);
	} else {
		math::plane sp = average_splitting_plane(node->aabb, props, node->axis);

		size_t reserve_size = props.size() / 2 + 1;
		std::vector<Prop*> props0;
		props0.reserve(reserve_size);
		std::vector<Prop*> props1;
		props1.reserve(reserve_size);

		for (auto p : props) {
			if (!p->shape()->is_finite() || p->shape()->is_delta()) {
				continue;
			}

			bool mib = math::behind(sp, p->aabb().min());
			bool mab = math::behind(sp, p->aabb().max());

			if (mib && mab) {
				props0.push_back(p);
			} else {
				props1.push_back(p);
			}
		}

		if (props0.empty()) {
			assign(node, props1, out_props);
		} else {
			node->children[0] = new Build_node;
			split(node->children[0], props0, max_shapes, out_props);

			node->children[1] = new Build_node;
			split(node->children[1], props1, max_shapes, out_props);
		}
	}
}

void Builder::assign(Build_node* node, const std::vector<Prop*>& props, std::vector<Prop*>& out_props) {
	node->offset = static_cast<uint32_t>(out_props.size());

	for (auto p : props) {
		if (!p->shape()->is_finite() || p->shape()->is_delta()) {
			continue;
		}

		out_props.push_back(p);
	}

	node->props_end = static_cast<uint32_t>(out_props.size());
}

math::aabb Builder::aabb(const std::vector<Prop*>& props) {
	math::aabb aabb = math::aabb::empty();

	for (auto p : props) {
		if (!p->shape()->is_finite() || p->shape()->is_delta()) {
			continue;
		}

		aabb = aabb.merge(p->aabb());
	}

	return aabb;
}

math::plane Builder::average_splitting_plane(const math::aabb& aabb, const std::vector<Prop*>& props, uint8_t& axis) {
	math::float3 average = math::float3::identity;

	uint32_t num = 0;
	for (auto p : props) {
		if (!p->shape()->is_finite() || p->shape()->is_delta()) {
			continue;
		}

		average += p->aabb().position();
		++num;
	}

	average /= static_cast<float>(num);

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

}}
