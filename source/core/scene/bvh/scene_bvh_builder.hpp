#pragma once

#include "scene_bvh_tree.hpp"
#include "base/math/plane.hpp"
#include <vector>
#include <cstddef>

namespace scene {

class Prop;

namespace bvh {

class Builder {
public:

	void build(Tree& tree, const std::vector<Prop*>& props);

private:

	static void split(Build_node* node, const std::vector<Prop*>& props, size_t max_shapes,
					  std::vector<Prop*>& out_props);

	static void assign(Build_node* node, const std::vector<Prop*>& props, std::vector<Prop*>& out_props);

	static math::aabb aabb(const std::vector<Prop*>& props);

	static math::plane average_splitting_plane(const math::aabb& aabb, const std::vector<Prop*>& props, uint8_t& axis);
};

}}
