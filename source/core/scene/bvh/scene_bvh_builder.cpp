#include "scene_bvh_builder.hpp"
#include "scene/prop/prop.hpp"
#include "base/math/plane.inl"
#include "base/math/bounding/aabb.inl"

namespace scene { namespace bvh {

void Builder::build(Tree& tree, std::vector<Prop*>& finite_props, const std::vector<Prop*>& infite_props) {
	tree.clear();

	tree.props_.reserve(finite_props.size() + infite_props.size());

	if (finite_props.empty()) {
		tree.root_.aabb = math::aabb::infinite();
	} else {
		split(&tree.root_, finite_props.begin(), finite_props.end(), 4, tree.props_);
	}

	tree.infinite_props_start_ = static_cast<uint32_t>(tree.props_.size());

	for (auto p : infite_props) {
		tree.props_.push_back(p);
	}

	tree.infinite_props_end_ = static_cast<uint32_t>(tree.props_.size());
}

void Builder::split(Build_node* node, index begin, index end, uint32_t max_shapes, std::vector<Prop*>& out_props) {
	node->aabb = aabb(begin, end);

	if (static_cast<uint32_t>(std::distance(begin, end)) <= max_shapes) {
		assign(node, begin, end, out_props);
	} else {
		Split_candidate sp = splitting_plane(node->aabb, begin, end);
		node->axis = sp.axis();

		index props1_begin = std::partition(begin, end,
			[&sp](Prop* p) {
				bool mib = math::behind(sp.plane(), p->aabb().min());
				bool mab = math::behind(sp.plane(), p->aabb().max());

				return mib && mab;
			});

		if (begin == props1_begin) {
			assign(node, props1_begin, end, out_props);
		} else {
			node->children[0] = new Build_node;
			split(node->children[0], begin, props1_begin, max_shapes, out_props);

			node->children[1] = new Build_node;
			split(node->children[1], props1_begin, end, max_shapes, out_props);
		}
	}
}

void Builder::assign(Build_node* node, index begin, index end, std::vector<Prop*>& out_props) {
	node->offset = static_cast<uint32_t>(out_props.size());

	for (index i = begin; i != end; ++i) {
		out_props.push_back(*i);
	}

	node->props_end = static_cast<uint32_t>(out_props.size());
}

math::aabb Builder::aabb(index begin, index end) {
	math::aabb aabb = math::aabb::empty();

	for (index i = begin; i != end; ++i) {
		aabb = aabb.merge((*i)->aabb());
	}

	return aabb;
}

Split_candidate Builder::splitting_plane(const math::aabb& /*aabb*/, index begin, index end) {
	split_candidates_.clear();

	math::float3 average = math::float3_identity;

	for (index i = begin; i != end; ++i) {
		average += (*i)->aabb().position();
	}

	average /= static_cast<float>(std::distance(begin, end));

	split_candidates_.push_back(Split_candidate(0, average, begin, end));

	split_candidates_.push_back(Split_candidate(1, average, begin, end));

	split_candidates_.push_back(Split_candidate(2, average, begin, end));

	std::sort(split_candidates_.begin(), split_candidates_.end(),
		[](const Split_candidate& a, const Split_candidate& b){ return a.key() < b.key(); });

	return split_candidates_[0];
}

}}
