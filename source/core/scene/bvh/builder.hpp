#pragma once

#include "tree.hpp"
#include <vector>

namespace scene {

class Prop;

namespace bvh {

class Builder {
public:

	void build(const std::vector<Prop*>& props, Tree& tree);
};

}}
