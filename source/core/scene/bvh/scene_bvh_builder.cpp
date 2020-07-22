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

        tree.alllocate_indices(num_references_);
        tree.allocate_nodes(uint32_t(build_nodes_.size()));

        serialize(tree, threads);
    }

    tree.aabb_ = build_nodes_[0].aabb();
}

void Builder::serialize(Tree& tree, thread::Pool& threads) const {
    threads.run_range(
        [this, &tree](uint32_t /*id*/, int32_t begin, int32_t end) noexcept {
            for (int32_t id = begin; id < end; ++id) {
                Build_node const& node = build_nodes_[id];

                auto& n = tree.nodes()[id];

                n.set_aabb(node.min().v, node.max().v);

                if (0xFFFFFFFF != node.children[0]) {
                    n.set_split_node(node.children[1], node.axis());
                } else {
                    uint32_t const i   = node.start_index();
                    uint8_t const  num = node.num_indices();
                    n.set_leaf_node(i, num);

                    uint32_t const* const primitives = node.primitives;

                    std::copy(primitives, primitives + num, &tree.indices_[i]);
                }
            }
        },
        0, int32_t(build_nodes_.size()));
}

}  // namespace scene::bvh
