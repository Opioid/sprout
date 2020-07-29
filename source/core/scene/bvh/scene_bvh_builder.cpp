#include "scene_bvh_builder.hpp"
#include "base/math/aabb.inl"
#include "base/memory/array.inl"
#include "base/thread/thread_pool.hpp"
#include "scene/scene.inl"
#include "scene_bvh_builder_base.inl"
#include "scene_bvh_node.inl"
#include "scene_bvh_split_candidate.inl"
#include "scene_bvh_tree.inl"

#include <algorithm>

namespace scene::bvh {

Builder::Builder() : Builder_base(16, 64, 4) {}

Builder::~Builder() = default;

void Builder::build(Tree& tree, std::vector<uint32_t>& indices, std::vector<AABB> const& aabbs,
                    thread::Pool& threads) {
    uint32_t const num_primitives = uint32_t(indices.size());

    reserve(num_primitives);

    if (indices.empty()) {
        build_nodes_[0].set_aabb({float3(-1.f), float3(1.f)});

        tree.alllocate_indices(0);
        tree.allocate_nodes(0);
    } else {
        {
            References references(num_primitives);

            memory::Array<Simd_AABB> taabbs(threads.num_threads() /*, AABB::empty()*/);

            threads.run_range(
                [&indices, &aabbs, &references, &taabbs](uint32_t id, int32_t begin,
                                                         int32_t end) noexcept {
                    Simd_AABB aabb(AABB::empty());

                    for (int32_t i = begin; i < end; ++i) {
                        uint32_t const prop = indices[uint32_t(i)];

                        Simd_AABB const b(aabbs[prop]);

                        references[uint32_t(i)].set(b.min, b.max, prop);

                        aabb.merge_assign(b);
                    }

                    taabbs[id] = aabb;
                    // taabbs[id].merge_assign(aabb);
                },
                0, int32_t(indices.size()));

            Simd_AABB aabb(AABB::empty());
            for (auto const& b : taabbs) {
                aabb.merge_assign(b);
            }

            split(references, AABB(aabb.min, aabb.max), threads);
        }

        tree.alllocate_indices(uint32_t(reference_ids_.size()));
        nodes_ = tree.allocate_nodes(uint32_t(build_nodes_.size()));

        uint32_t current_prop = 0;
        new_node();
        serialize(0, 0, tree, current_prop);
    }

    tree.aabb_ = build_nodes_[0].aabb();
}

void Builder::serialize(uint32_t source_node, uint32_t dest_node, Tree& tree,
                        uint32_t& current_prop) {
    Build_node const& node = build_nodes_[source_node];

    auto& n = nodes_[dest_node];
    n.set_aabb(node.min().v, node.max().v);

    if (0 == node.max_.num_indices) {
        uint32_t const child0 = current_node_index();

        n.set_split_node(child0, node.axis());

        new_node();
        new_node();

        uint32_t const source_child0 = node.min_.children_or_data;

        serialize(source_child0, child0, tree, current_prop);

        serialize(source_child0 + 1, child0 + 1, tree, current_prop);
    } else {
        uint32_t const i   = current_prop;
        uint8_t const  num = node.num_indices();
        n.set_leaf_node(i, num);

        uint32_t const* begin = &reference_ids_[node.min_.children_or_data];
        uint32_t const* end   = begin + num;
        std::copy(begin, end, &tree.indices_[i]);

        current_prop += num;
    }
}

}  // namespace scene::bvh
