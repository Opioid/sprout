#ifndef SU_CORE_SCENE_BVH_TREE_INL
#define SU_CORE_SCENE_BVH_TREE_INL

#include "base/memory/align.hpp"
#include "scene_bvh_node.hpp"
#include "scene_bvh_tree.hpp"

namespace scene::bvh {

template <typename T>
Tree<T>::~Tree() noexcept {
    memory::free_aligned(nodes_);
}

template <typename T>
bvh::Node* Tree<T>::allocate_nodes(uint32_t num_nodes) noexcept {
    if (num_nodes != num_nodes_) {
        num_nodes_ = num_nodes;

        memory::free_aligned(nodes_);
        nodes_ = memory::allocate_aligned<Node>(num_nodes);
    }

    return nodes_;
}

}  // namespace scene::bvh

#endif
