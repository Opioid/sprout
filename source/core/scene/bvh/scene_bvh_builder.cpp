#include "scene_bvh_builder.hpp"
#include "base/math/aabb.inl"
#include "base/memory/array.inl"
#include "base/thread/thread_pool.hpp"
#include "scene/scene.inl"
#include "scene_bvh_node.inl"
#include "scene_bvh_split_candidate.inl"
#include "scene_bvh_tree.inl"

namespace scene::bvh {

Builder::Builder() : Builder_base(16, 64) {}

Builder::~Builder() = default;

void Builder::build(Tree& tree, std::vector<uint32_t>& indices, std::vector<AABB> const& aabbs,
                    thread::Pool& threads) {
    Build_node root;

    if (indices.empty()) {
        root.aabb = AABB(float3(-1.f), float3(1.f));

        tree.alllocate_indices(0);

        nodes_ = tree.allocate_nodes(0);
    } else {
        {
            float const log2_num_triangles = std::log2(float(indices.size()));

            spatial_split_threshold_ = uint32_t(std::lrint(log2_num_triangles / 2.f));

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
            for (auto& b : taabbs) {
                aabb.merge_assign(b);
            }

            num_nodes_      = 1;
            num_references_ = 0;

            split(&root, references, AABB(aabb.min, aabb.max), 4, 0, threads);
        }

        tree.alllocate_indices(num_references_);

        nodes_ = tree.allocate_nodes(num_nodes_);

        current_node_ = 0;

        uint32_t current_prop = 0;
        serialize(&root, tree, current_prop);
    }

    tree.aabb_ = root.aabb;
}

void Builder::serialize(Build_node* node, Tree& tree, uint32_t& current_prop) {
    auto& n = new_node();
    n.set_aabb(node->aabb.min().v, node->aabb.max().v);

    if (node->children[0]) {
        serialize(node->children[0], tree, current_prop);

        n.set_split_node(current_node_index(), node->axis);

        serialize(node->children[1], tree, current_prop);
    } else {
        uint8_t const num_primitives = uint8_t(node->end_index - node->start_index);
        n.set_leaf_node(node->start_index, num_primitives);

        uint32_t i = current_prop;
        for (auto const p : node->primitives) {
            tree.indices_[i] = p;
            ++i;
        }

        current_prop = i;
    }
}

}  // namespace scene::bvh
