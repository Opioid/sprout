#include "builder.hpp"
#include "scene/prop/prop.hpp"
#include "scene/shape/shape.hpp"
#include <iostream>

namespace scene { namespace bvh {

void Builder::build(Tree& tree, const std::vector<Prop*>& props) {
	tree.props_.clear();

	split(tree.root_, props, 4, tree.props_);
}

void Builder::split(Build_node& node, const std::vector<Prop*>& props, size_t max_shapes, std::vector<Prop*>& out_props) {
	node.aabb = aabb(props);

	std::cout << props.size() << std::endl;

	if (props.size() >= max_shapes) {
		assign(node, props, out_props);
	} else {
		node.children[0] = new Build_node;
		node.children[1] = new Build_node;
	}
}

void Builder::assign(Build_node& node, const std::vector<Prop*>& props, std::vector<Prop*>& out_props) {

}

math::AABB Builder::aabb(const std::vector<Prop*>& props) {
	math::AABB aabb = math::AABB::empty();

	for (auto p : props) {
		if (!p->shape()->is_finite()) {
			continue;
		}

		aabb = aabb.merge(p->aabb());
	}

	return aabb;
}

}}
