#include "scene_bvh_builder.hpp"
#include "base/math/aabb.inl"
#include "base/memory/array.inl"
#include "base/thread/thread_pool.hpp"
#include "scene/scene.inl"
#include "scene_bvh_node.inl"
#include "scene_bvh_split_candidate.inl"
#include "scene_bvh_tree.inl"

namespace scene::bvh {

Builder::Builder() : Builder_base(16, 64, 4) {}

Builder::~Builder() = default;

void Builder::build(Tree& tree, std::vector<uint32_t>& indices, std::vector<AABB> const& aabbs,
                    thread::Pool& threads) {
    build_nodes_.reserve(std::max((3 * uint32_t(indices.size())) / max_primitives_, 1u));
    build_nodes_.clear();
    build_nodes_.emplace_back();

    if (indices.empty()) {
        build_nodes_[0].aabb = AABB(float3(-1.f), float3(1.f));

        tree.alllocate_indices(0);
        tree.allocate_nodes(0);
    } else {
        {
            float const log2_num_primitives = std::log2(float(indices.size()));

            spatial_split_threshold_ = uint32_t(std::lrint(log2_num_primitives / 2.f));

            References references(indices.size());

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

            num_references_ = 0;

            split(0, references, AABB(aabb.min, aabb.max), 0, threads);
        }

        tree.alllocate_indices(num_references_);
        tree.allocate_nodes(uint32_t(build_nodes_.size()));

        serialize(tree);
    }

    tree.aabb_ = build_nodes_[0].aabb;
}

void Builder::serialize(Tree& tree) const {
    for (uint32_t id = 0, i = 0, num_nodes = build_nodes_.size(); id < num_nodes; ++id) {
        Build_node const& node = build_nodes_[id];

        auto& n = tree.nodes()[id];

        n.set_aabb(node.aabb.min().v, node.aabb.max().v);

        if (0xFFFFFFFF != node.children[0]) {
            n.set_split_node(node.children[1], node.axis);
        } else {
            uint8_t const num_primitives = node.num_indices;
            n.set_leaf_node(node.start_index, num_primitives);

            uint32_t const* const primitives = node.primitives;

            for (uint32_t p = 0, len = node.num_indices; p < len; ++p, ++i) {
                tree.indices_[i] = primitives[p];
            }
        }
    }
}

}  // namespace scene::bvh
