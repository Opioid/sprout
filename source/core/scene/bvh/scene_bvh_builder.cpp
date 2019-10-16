#include "scene_bvh_builder.hpp"
#include "base/math/aabb.inl"
#include "base/math/plane.inl"
#include "scene_bvh_node.inl"
#include "scene_bvh_split_candidate.inl"
#include "scene_bvh_tree.inl"

namespace scene::bvh {

Builder::Builder() noexcept : root_(new Build_node) {
    root_->clear();
}

Builder::~Builder() noexcept {
    delete root_;
}

void Builder::build(Tree& tree, std::vector<uint32_t>& indices,
                    std::vector<AABB> const& aabbs) noexcept {
    if (indices.empty()) {
        nodes_ = tree.allocate_nodes(0);
    } else {
        num_nodes_ = 1;
        split(root_, indices.begin(), indices.end(), indices.begin(), aabbs, 4);

        nodes_ = tree.allocate_nodes(num_nodes_);

        current_node_ = 0;
        serialize(root_);
    }

    tree.aabb_ = root_->aabb;

    root_->clear();
}

Builder::Build_node::~Build_node() noexcept {
    delete children[0];
    delete children[1];
}

void Builder::Build_node::clear() noexcept {
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

void Builder::split(Build_node* node, index begin, index end, const_index origin,
                    std::vector<AABB> const& aabbs, uint32_t max_shapes) noexcept {
    node->aabb = aabb(begin, end, aabbs);

    if (uint32_t(std::distance(begin, end)) <= max_shapes) {
        assign(node, begin, end, origin);
    } else {
        Split_candidate sp = splitting_plane(node->aabb, begin, end, aabbs);

        node->axis = sp.axis();

        index props1_begin = std::partition(begin, end, [&sp, &aabbs](uint32_t b) {
            AABB const& p = aabbs[b];

            bool const mib = sp.behind(p.min());
            bool const mab = sp.behind(p.max());

            return mib && mab;
        });

        if (begin == props1_begin) {
            assign(node, props1_begin, end, origin);
        } else {
            node->children[0] = new Build_node;
            split(node->children[0], begin, props1_begin, origin, aabbs, max_shapes);

            node->children[1] = new Build_node;
            split(node->children[1], props1_begin, end, origin, aabbs, max_shapes);

            num_nodes_ += 2;
        }
    }
}

Split_candidate Builder::splitting_plane(AABB const& /*aabb*/, index begin, index end,
                                         std::vector<AABB> const& aabbs) noexcept {
    split_candidates_.clear();

    float3 average = float3(0.f);

    for (index i = begin; i != end; ++i) {
        average += aabbs[*i].position();
    }

    average /= float(std::distance(begin, end));

    split_candidates_.emplace_back(uint8_t(0), average, begin, end, aabbs);
    split_candidates_.emplace_back(uint8_t(1), average, begin, end, aabbs);
    split_candidates_.emplace_back(uint8_t(2), average, begin, end, aabbs);

    std::sort(split_candidates_.begin(), split_candidates_.end(),
              [](Split_candidate const& a, Split_candidate const& b) { return a.key() < b.key(); });

    return split_candidates_[0];
}

void Builder::serialize(Build_node* node) noexcept {
    auto& n = new_node();
    n.set_aabb(node->aabb.min().v, node->aabb.max().v);

    if (node->children[0]) {
        serialize(node->children[0]);

        n.set_split_node(current_node_index(), node->axis);

        serialize(node->children[1]);
    } else {
        uint8_t const num_primitives = uint8_t(node->props_end - node->offset);
        n.set_leaf_node(node->offset, num_primitives);
    }
}

bvh::Node& Builder::new_node() noexcept {
    return nodes_[current_node_++];
}

uint32_t Builder::current_node_index() const noexcept {
    return current_node_;
}

void Builder::assign(Build_node* node, const_index begin, const_index end,
                     const_index origin) noexcept {
    node->offset = uint32_t(begin - origin);

    node->props_end = uint32_t(end - origin);
}

AABB Builder::aabb(index begin, index end, std::vector<AABB> const& aabbs) noexcept {
    AABB aabb = AABB::empty();

    for (index i = begin; i != end; ++i) {
        aabb.merge_assign(aabbs[*i]);
    }

    return aabb;
}

}  // namespace scene::bvh
