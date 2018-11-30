#include "triangle_bvh_builder_base.hpp"
#include "base/math/aabb.inl"
#include "base/math/vector3.inl"
#include "scene/shape/shape_vertex.hpp"
#include "scene/shape/triangle/triangle_primitive.hpp"
#include "triangle_bvh_helper.hpp"
#include "triangle_bvh_tree.inl"

namespace scene::shape::triangle::bvh {

Build_node::Build_node() : start_index(0), end_index(0) {
    children[0] = nullptr;
    children[1] = nullptr;
}

Build_node::~Build_node() {
    delete children[0];
    delete children[1];
}

void Build_node::num_sub_nodes(uint32_t& count) {
    if (children[0]) {
        count += 2;

        children[0]->num_sub_nodes(count);
        children[1]->num_sub_nodes(count);
    }
}

void Builder_base::serialize(Build_node* node) {
    auto& n = new_node();
    n.set_aabb(node->aabb.min().v, node->aabb.max().v);

    if (node->children[0]) {
        serialize(node->children[0]);

        n.set_split_node(current_node_index(), node->axis);

        serialize(node->children[1]);
    } else {
        uint8_t num_primitives = static_cast<uint8_t>(node->end_index - node->start_index);
        n.set_leaf_node(node->start_index, num_primitives);
    }
}

scene::bvh::Node& Builder_base::new_node() {
    return nodes_[current_node_++];
}

uint32_t Builder_base::current_node_index() const {
    return current_node_;
}

AABB Builder_base::submesh_aabb(index begin, index end,
                                const std::vector<Index_triangle>& triangles,
                                const std::vector<Vertex>&         vertices) {
    float  max_float = std::numeric_limits<float>::max();
    float3 min(max_float, max_float, max_float);
    float3 max(-max_float, -max_float, -max_float);

    for (index i = begin; i != end; ++i) {
        auto const& t = triangles[*i];

        auto const a = float3(vertices[t.i[0]].p);
        auto const b = float3(vertices[t.i[1]].p);
        auto const c = float3(vertices[t.i[2]].p);

        min = triangle_min(a, b, c, min);
        max = triangle_max(a, b, c, max);
    }

    //	float constexpr epsilon = 0.000000001f;

    //	max.x += epsilon;
    //	max.y += epsilon;
    //	max.z += epsilon;

    return AABB(min, max);
}

}  // namespace scene::shape::triangle::bvh
