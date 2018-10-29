#ifndef SU_CORE_SCENE_BVH_TREE_HPP
#define SU_CORE_SCENE_BVH_TREE_HPP

#include <vector>
#include "base/math/aabb.hpp"

namespace scene::bvh {

class Node;

template <typename T>
struct Tree {
    Tree() noexcept = default;

    ~Tree() noexcept;

    void clear() noexcept;

    using Node = bvh::Node;

    Node* allocate_nodes(uint32_t num_nodes) noexcept;

    uint32_t num_nodes_ = 0;
    Node*    nodes_     = nullptr;

    std::vector<T*> data_;

    math::AABB aabb_;
};

}  // namespace scene::bvh

#endif
