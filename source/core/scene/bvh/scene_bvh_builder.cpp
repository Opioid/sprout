#include "scene_bvh_builder.hpp"
#include "scene/prop/prop.hpp"
//#include "scene/shape/shape.hpp"
#include "base/math/plane.inl"
#include "base/math/bounding/aabb.inl"

#include <iostream>

namespace scene { namespace bvh {

void Builder::build(Tree& tree, const std::vector<Prop*>& finite_props, const std::vector<Prop*>& infite_props) {
	tree.clear();

	tree.props_.reserve(finite_props.size() + infite_props.size());

	split(&tree.root_, finite_props, 4, tree.props_);

	tree.infinite_props_start_ = static_cast<uint32_t>(tree.props_.size());

	for (auto p : infite_props) {
		tree.props_.push_back(p);
	}

	tree.infinite_props_end_ = static_cast<uint32_t>(tree.props_.size());
}

void Builder::split(Build_node* node, const std::vector<Prop*>& props, size_t max_shapes,
					std::vector<Prop*>& out_props) {
	node->aabb = aabb(props);

	if (props.size() <= max_shapes) {
		assign(node, props, out_props);
	} else {
	//	math::plane sp = average_splitting_plane(node->aabb, props, node->axis);

		Split_candidate sp = splitting_plane(node->aabb, props);
		node->axis = sp.axis();

		size_t reserve_size = props.size() / 2 + 1;
		std::vector<Prop*> props0;
		props0.reserve(reserve_size);
		std::vector<Prop*> props1;
		props1.reserve(reserve_size);

		for (auto p : props) {
			bool mib = math::behind(sp.plane(), p->aabb().min());
			bool mab = math::behind(sp.plane(), p->aabb().max());

			if (mib && mab) {
				props0.push_back(p);
			} else {
				props1.push_back(p);
			}
		}

		if (props0.empty()) {
			//std::cout << "not splitting because of stuff: " << props1.size() << std::endl;

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
		out_props.push_back(p);
	}

	node->props_end = static_cast<uint32_t>(out_props.size());
}

math::aabb Builder::aabb(const std::vector<Prop*>& props) {
	math::aabb aabb = math::aabb::empty();

	for (auto p : props) {
		aabb = aabb.merge(p->aabb());
	}

	return aabb;
}

Split_candidate Builder::splitting_plane(const math::aabb& /*aabb*/, const std::vector<Prop*>& props) {
	split_candidates_.clear();

	math::float3 average = math::float3::identity;

	for (auto p : props) {
		average += p->aabb().position();
	}

	average /= static_cast<float>(props.size());

	split_candidates_.push_back(Split_candidate(0, average, props));

	split_candidates_.push_back(Split_candidate(1, average, props));

	split_candidates_.push_back(Split_candidate(2, average, props));

	std::sort(split_candidates_.begin(), split_candidates_.end(),
			  [](const Split_candidate& a, const Split_candidate& b){ return a.key() < b.key(); });

	return split_candidates_[0];
}

math::plane Builder::average_splitting_plane(const math::aabb& aabb, const std::vector<Prop*>& props, uint8_t& axis) {
	math::float3 average = math::float3::identity;

	for (auto p : props) {
		average += p->aabb().position();
	}

	average /= static_cast<float>(props.size());

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
