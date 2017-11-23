#ifndef SU_CORE_SCENE_BVH_BUILDER_HPP
#define SU_CORE_SCENE_BVH_BUILDER_HPP

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

	Builder();
	~Builder();

	void build(Tree& tree, std::vector<Prop*>& finite_props,
			   const std::vector<Prop*>& infite_props);

private:

	struct Build_node {
		Build_node() = default;
		~Build_node();

		void clear();

		math::AABB aabb;

		uint8_t axis;

		uint32_t offset;
		uint32_t props_end;

		Build_node* children[2] = { nullptr, nullptr };
	};

	using index = std::vector<Prop*>::iterator;

	void split(Build_node* node, index begin, index end, uint32_t max_shapes,
			   std::vector<Prop*>& out_props);

	Split_candidate splitting_plane(const math::AABB& aabb, index begin, index end);

	void serialize(Build_node* node);

	Node& new_node();

	uint32_t current_node_index() const;

	static void assign(Build_node* node, index begin, index end, std::vector<Prop*>& out_props);

	static math::AABB aabb(index begin, index end);

	std::vector<Split_candidate> split_candidates_;

	Build_node* root_;

	uint32_t num_nodes_;
	uint32_t current_node_;

	Node* nodes_;
};

}}

#endif
