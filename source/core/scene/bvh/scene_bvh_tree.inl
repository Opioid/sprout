#ifndef SU_CORE_SCENE_BVH_TREE_INL
#define SU_CORE_SCENE_BVH_TREE_INL

#include "scene_bvh_node.hpp"
#include "scene_bvh_tree.hpp"

namespace scene::bvh {

inline Tree::Tree() = default;

inline Tree::~Tree() {
    delete[] indices_;
    delete[] nodes_;
}

inline bvh::Node* Tree::allocate_nodes(uint32_t num_nodes) {
    if (num_nodes != num_nodes_) {
        num_nodes_ = num_nodes;

        delete[] nodes_;
        nodes_ = new Node[num_nodes];
    }

    return nodes_;
}

inline void Tree::alllocate_indices(uint32_t num_indices) {
    if (num_indices != num_indices_) {
        num_indices_ = num_indices;

        delete[] indices_;
        indices_ = new uint32_t[num_indices];
    }
}

}  // namespace scene::bvh

#endif
