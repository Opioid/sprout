#ifndef SU_CORE_SCENE_BVH_TREE_HPP
#define SU_CORE_SCENE_BVH_TREE_HPP

#include "base/math/aabb.hpp"

namespace scene::bvh {

class Node;

struct Tree {
    Tree();

    ~Tree();

    using Node = bvh::Node;

    Node* allocate_nodes(uint32_t num_nodes);

    void alllocate_indices(uint32_t num_indices);

    uint32_t num_nodes_   = 0;
    uint32_t num_indices_ = 0;

    Node* nodes_ = nullptr;

    uint32_t* indices_ = nullptr;
};

}  // namespace scene::bvh

#endif
