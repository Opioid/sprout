#pragma once

#include "scene_bvh_split_candidate.hpp"
#include "scene_bvh_tree.hpp"
#include "base/math/plane.hpp"
#include <vector>
#include <cstddef>

namespace scene {

class Prop;

namespace bvh {

class Builder {
public:

	void build(Tree& tree, std::vector<Prop*>& finite_props, const std::vector<Prop*>& infite_props);

private:

	typedef std::vector<Prop*>::iterator index;

	void split(Build_node* node, index begin, index end, uint32_t max_shapes, std::vector<Prop*>& out_props);

	static void assign(Build_node* node, index begin, index end, std::vector<Prop*>& out_props);

	static math::aabb aabb(index begin, index end);

	Split_candidate splitting_plane(const math::aabb& aabb, index begin, index end);

	std::vector<Split_candidate> split_candidates_;
};

}}
