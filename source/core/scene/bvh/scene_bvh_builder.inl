#ifndef SU_CORE_SCENE_BVH_BUILDER_INL
#define SU_CORE_SCENE_BVH_BUILDER_INL

#include "base/math/aabb.inl"
#include "base/math/plane.inl"
#include "scene_bvh_builder.hpp"
#include "scene_bvh_node.inl"
#include "scene_bvh_split_candidate.inl"
#include "scene_bvh_tree.inl"

#include <iostream>

namespace scene::bvh {

template <typename T>
Builder<T>::Builder() noexcept : root_(new Build_node) {
    root_->clear();
}

template <typename T>
Builder<T>::~Builder() noexcept {
    delete root_;
}

template <typename T>
void Builder<T>::build(Tree<T>& tree, std::vector<uint32_t>& indices, std::vector<T> const& props) noexcept {
    if (props.empty()) {
        nodes_ = tree.allocate_nodes(0);
    } else {
        num_nodes_ = 1;
        split(root_, indices.begin(), indices.end(), indices.begin(), props, 4);

        nodes_ = tree.allocate_nodes(num_nodes_);

        current_node_ = 0;
        serialize(root_);
    }

    tree.aabb_ = root_->aabb;

    root_->clear();
}

template <typename T>
Builder<T>::Build_node::~Build_node() noexcept {
    delete children[0];
    delete children[1];
}

template <typename T>
void Builder<T>::Build_node::clear() noexcept {
    delete children[0];
    children[0] = nullptr;
    delete children[1];
    children[1] = nullptr;

    props_end = 0;
    offset    = 0;

    // This size will be used even if there are only infinite props in the scene.
    // It is an arbitrary size that will be used to calculate the power of some lights.
    aabb = AABB(float3(-1.f), float3(1.f));
}

template <typename T>
void Builder<T>::split(Build_node* node, index begin, index end, const_index origin, std::vector<T> const& props,
                       uint32_t max_shapes) noexcept {
    node->aabb = aabb(begin, end, props);

    if (static_cast<uint32_t>(std::distance(begin, end)) <= max_shapes) {
        assign(node, begin, end, origin);
    } else {
        Split_candidate<T> sp = splitting_plane(node->aabb, begin, end, props);

        node->axis = sp.axis();

        index props1_begin = std::partition(begin, end, [&sp, &props](uint32_t b) {
            T const& p = props[b];
            bool const mib = math::plane::behind(sp.plane(), p.aabb().min());
            bool const mab = math::plane::behind(sp.plane(), p.aabb().max());

            return mib && mab;
        });

        if (begin == props1_begin) {
            assign(node, props1_begin, end, origin);
        } else {
            node->children[0] = new Build_node;
            split(node->children[0], begin, props1_begin, origin, props, max_shapes);

            node->children[1] = new Build_node;
            split(node->children[1], props1_begin, end, origin, props, max_shapes);

            num_nodes_ += 2;
        }
    }
}

template <typename T>
Split_candidate<T> Builder<T>::splitting_plane(AABB const& /*aabb*/, index begin,
                                               index end, std::vector<T> const& props) noexcept {
    split_candidates_.clear();

    float3 average = float3(0.f);

    for (index i = begin; i != end; ++i) {
        average += props[*i].aabb().position();
    }

    average /= static_cast<float>(std::distance(begin, end));

    split_candidates_.emplace_back(uint8_t(0), average, begin, end, props);
    split_candidates_.emplace_back(uint8_t(1), average, begin, end, props);
    split_candidates_.emplace_back(uint8_t(2), average, begin, end, props);

    std::sort(
        split_candidates_.begin(), split_candidates_.end(),
        [](Split_candidate<T> const& a, Split_candidate<T> const& b) { return a.key() < b.key(); });

    return split_candidates_[0];
}

template <typename T>
void Builder<T>::serialize(Build_node* node) noexcept {
    auto& n = new_node();
    n.set_aabb(node->aabb.min().v, node->aabb.max().v);

    if (node->children[0]) {
        serialize(node->children[0]);

        n.set_split_node(current_node_index(), node->axis);

        serialize(node->children[1]);
    } else {
        uint8_t const num_primitives = static_cast<uint8_t>(node->props_end - node->offset);
        n.set_leaf_node(node->offset, num_primitives);
    }
}

template <typename T>
bvh::Node& Builder<T>::new_node() noexcept {
    return nodes_[current_node_++];
}

template <typename T>
uint32_t Builder<T>::current_node_index() const noexcept {
    return current_node_;
}

template <typename T>
void Builder<T>::assign(Build_node* node, const_index begin, const_index end,
                        const_index origin) noexcept {
    node->offset = static_cast<uint32_t>(std::distance(origin, begin));

    node->props_end = static_cast<uint32_t>(std::distance(origin, end));

    std::cout << node->offset << " " << node->props_end << std::endl;
}

template <typename T>
AABB Builder<T>::aabb(index begin, index end, std::vector<T> const& props) noexcept {
    AABB aabb = AABB::empty();

    for (index i = begin; i != end; ++i) {
        aabb.merge_assign(props[*i].aabb());
    }

    return aabb;
}

}  // namespace scene::bvh

#endif
