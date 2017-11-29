#include "scene_bvh_tree.hpp"
#include "scene_bvh_node.hpp"
#include "base/memory/align.hpp"

namespace scene::bvh {

Tree::~Tree() {
	memory::free_aligned(nodes_);
}

void Tree::clear() {
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

}
