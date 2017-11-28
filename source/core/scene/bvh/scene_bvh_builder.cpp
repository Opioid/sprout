#include "scene_bvh_builder.hpp"
#include "scene_bvh_node.inl"
#include "scene/prop/prop.hpp"
#include "base/math/aabb.inl"
#include "base/math/plane.inl"

namespace scene::bvh {

Builder::Builder() : root_(new Build_node) {
	root_->clear();
}

Builder::~Builder() {
	delete root_;
}

void Builder::build(Tree& tree, std::vector<Prop*>& finite_props,
					const std::vector<Prop*>& infite_props) {
	tree.clear();

	tree.props_.reserve(finite_props.size());

	if (finite_props.empty()) {
		nodes_ = tree.allocate_nodes(0);
	} else {
		num_nodes_ = 1;
		split(root_, finite_props.begin(), finite_props.end(), 4, tree.props_);

		nodes_ = tree.allocate_nodes(num_nodes_);

		current_node_ = 0;
		serialize(root_);
	}

	tree.num_infinite_props_ = static_cast<uint32_t>(infite_props.size());
	tree.infinite_props_	 = infite_props.data();

	tree.aabb_ = root_->aabb;

	root_->clear();
}

Builder::Build_node::~Build_node() {
	delete children[0];
	delete children[1];
}

void Builder::Build_node::clear() {
	delete children[0];
	children[0] = nullptr;
	delete children[1];
	children[1] = nullptr;

	props_end = 0;
	offset = 0;

	// This size will be used even if there are only infinite props in the scene.
	// It is an arbitrary size that will be used to calculate the power of some lights.
	aabb = math::AABB(float3(-1.f), float3(1.f));
}

void Builder::split(Build_node* node, index begin, index end, uint32_t max_shapes,
					std::vector<Prop*>& out_props) {
	node->aabb = aabb(begin, end);

	if (static_cast<uint32_t>(std::distance(begin, end)) <= max_shapes) {
		assign(node, begin, end, out_props);
	} else {
		Split_candidate sp = splitting_plane(node->aabb, begin, end);
		node->axis = sp.axis();

		index props1_begin = std::partition(begin, end, [&sp](Prop* p) {
			bool mib = math::plane::behind(sp.plane(), p->aabb().min());
			bool mab = math::plane::behind(sp.plane(), p->aabb().max());

			return mib && mab;
		});

		if (begin == props1_begin) {
			assign(node, props1_begin, end, out_props);
		} else {
			node->children[0] = new Build_node;
			split(node->children[0], begin, props1_begin, max_shapes, out_props);

			node->children[1] = new Build_node;
			split(node->children[1], props1_begin, end, max_shapes, out_props);

			num_nodes_ += 2;
		}
	}
}

Split_candidate Builder::splitting_plane(const math::AABB& /*aabb*/, index begin, index end) {
	split_candidates_.clear();

	float3 average = float3::identity();

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

void Builder::serialize(Build_node* node) {
	auto& n = new_node();
	n.set_aabb(node->aabb.min().v, node->aabb.max().v);

	if (node->children[0]) {
		serialize(node->children[0]);

		n.set_split_node(current_node_index(), node->axis);

		serialize(node->children[1]);
	} else {
		uint8_t num_primitives = static_cast<uint8_t>(node->props_end - node->offset);
		n.set_leaf_node(node->offset, num_primitives);
	}
}

bvh::Node& Builder::new_node() {
	return nodes_[current_node_++];
}

uint32_t Builder::current_node_index() const {
	return current_node_;
}

void Builder::assign(Build_node* node, index begin, index end, std::vector<Prop*>& out_props) {
	node->offset = static_cast<uint32_t>(out_props.size());

	for (index i = begin; i != end; ++i) {
		out_props.push_back(*i);
	}

	node->props_end = static_cast<uint32_t>(out_props.size());
}

math::AABB Builder::aabb(index begin, index end) {
	math::AABB aabb = math::AABB::empty();

	for (index i = begin; i != end; ++i) {
		aabb = aabb.merge((*i)->aabb());
	}

	return aabb;
}

}
