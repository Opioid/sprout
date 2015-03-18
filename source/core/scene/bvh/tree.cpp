#include "tree.hpp"

namespace scene { namespace bvh {

Build_node::Build_node() {
	children[0] = nullptr;
	children[1] = nullptr;
}

Build_node::~Build_node() {
	delete children[0];
	delete children[1];
}


bool Tree::intersect_p() const {
	return false;
}

}}
