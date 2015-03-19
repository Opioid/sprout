#pragma once

#include "tree.hpp"
#include "base/math/plane.hpp"
#include <vector>

namespace scene {

class Prop;

namespace bvh {

class Builder {
public:

	void build(Tree& tree, const std::vector<Prop*>& props);

private:

	static void split(Build_node* node, const std::vector<Prop*>& props, size_t max_shapes, std::vector<Prop*>& out_props);
	static void assign(Build_node* node, const std::vector<Prop*>& props, std::vector<Prop*>& out_props);

	static math::AABB aabb(const std::vector<Prop*>& props);

	static math::plane choose_average_splitting_plane(const math::AABB aabb, const std::vector<Prop*>& props, uint32_t& axis);
};

}}
