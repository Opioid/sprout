#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>
#include "base/math/aabb.hpp"

namespace scene {

namespace bvh {
class Node;
}

namespace shape {

struct Vertex;

namespace triangle {

struct Index_triangle;
struct Triangle;

namespace bvh {

template <typename Data>
class Tree;

struct Build_node {
    Build_node();
    ~Build_node();

    void num_sub_nodes(uint32_t& count);

    AABB aabb;

    uint32_t start_index;
    uint32_t end_index;

    uint8_t axis;

    Build_node* children[2];
};

class Builder_base {
  protected:
    using Node  = scene::bvh::Node;
    using index = std::vector<uint32_t>::iterator;

    void serialize(Build_node* node);

    Node& new_node();

    uint32_t current_node_index() const;

    static AABB submesh_aabb(index begin, index end, const std::vector<Index_triangle>& triangles,
                             const std::vector<Vertex>& vertices);

    uint32_t num_nodes_;
    uint32_t current_node_;

    Node* nodes_;
};

}  // namespace bvh
}  // namespace triangle
}  // namespace shape
}  // namespace scene
