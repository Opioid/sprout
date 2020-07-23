#ifndef SU_CORE_SCENE_BVH_TREE_INL
#define SU_CORE_SCENE_BVH_TREE_INL

#include "base/memory/align.hpp"
#include "scene_bvh_node.hpp"
#include "scene_bvh_tree.hpp"

namespace scene::bvh {

inline Tree::Tree() = default;

inline Tree::~Tree() {
    memory::free_aligned(indices_);
    memory::free_aligned(nodes_);
}

inline bvh::Node* Tree::allocate_nodes(uint32_t num_nodes) {
    if (num_nodes != num_nodes_) {
        num_nodes_ = num_nodes;

        memory::free_aligned(nodes_);
        nodes_ = memory::allocate_aligned<Node>(num_nodes);
    }

    return nodes_;
}

inline void Tree::alllocate_indices(uint32_t num_indices) {
    if (num_indices != num_indices_) {
        num_indices_ = num_indices;

        memory::free_aligned(indices_);
        indices_ = memory::allocate_aligned<uint32_t>(num_indices);
    }
}

}  // namespace scene::bvh

#endif
