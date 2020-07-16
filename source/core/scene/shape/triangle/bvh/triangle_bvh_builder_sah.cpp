#include "triangle_bvh_builder_sah.hpp"
#include "base/math/aabb.inl"
#include "base/math/vector3.inl"
#include "base/memory/array.inl"
#include "base/thread/thread_pool.hpp"
#include "logging/logging.hpp"
#include "scene/bvh/scene_bvh_builder_base.inl"
#include "scene/bvh/scene_bvh_node.inl"
#include "scene/bvh/scene_bvh_split_candidate.inl"
#include "scene/shape/shape_vertex.hpp"
#include "scene/shape/triangle/triangle_primitive.hpp"
#include "triangle_bvh_helper.hpp"
#include "triangle_bvh_tree.inl"

namespace scene::shape::triangle::bvh {

Builder_SAH::Builder_SAH(uint32_t num_slices, uint32_t sweep_threshold, uint32_t max_primitives)
    : Builder_base(num_slices, sweep_threshold, max_primitives) {}

Builder_SAH::~Builder_SAH() = default;

void Builder_SAH::build(triangle::Tree& tree, uint32_t num_triangles, Triangles triangles,
                        Vertices vertices, thread::Pool& threads) {
    build_nodes_.reserve((3 * num_triangles) / max_primitives_);
    build_nodes_.clear();
    build_nodes_.emplace_back();

    {
        float const log2_num_triangles = std::log2(float(num_triangles));

        spatial_split_threshold_ = uint32_t(std::lrint(log2_num_triangles / 2.f));

        References references(num_triangles);

        memory::Array<Simd_AABB> aabbs(threads.num_threads() /*, AABB::empty()*/);

        threads.run_range(
            [&triangles, &vertices, &references, &aabbs](uint32_t id, int32_t begin,
                                                         int32_t end) noexcept {
                Simd_AABB aabb(AABB::empty());

                for (int32_t i = begin; i < end; ++i) {
                    auto const a = Simd3f(vertices.p(triangles[i].i[0]));
                    auto const b = Simd3f(vertices.p(triangles[i].i[1]));
                    auto const c = Simd3f(vertices.p(triangles[i].i[2]));

                    auto const min = triangle_min(a, b, c);
                    auto const max = triangle_max(a, b, c);

                    uint32_t const ui = uint32_t(i);
                    references[ui].set(min, max, ui);

                    aabb.merge_assign(min, max);
                }

                aabbs[id] = aabb;
                // aabbs[id].merge_assign(aabb);
            },
            0, int32_t(num_triangles));

        Simd_AABB aabb(AABB::empty());
        for (auto const& b : aabbs) {
            aabb.merge_assign(b);
        }

        num_references_ = 0;

        split(0, references, AABB(aabb.min, aabb.max), 0, threads);
    }

    tree.allocate_triangles(num_references_, vertices);
    tree.allocate_nodes(uint32_t(build_nodes_.size()));

    serialize(triangles, vertices, tree, threads);
}

void Builder_SAH::serialize(Triangles triangles, Vertices vertices, triangle::Tree& tree, thread::Pool& threads) const {
    threads.run_range([this, triangles, &vertices, &tree](uint32_t /*id*/, int32_t begin, int32_t end) noexcept {
        for (int32_t id = begin; id < end; ++id) {
            Build_node const& node = build_nodes_[uint32_t(id)];

            auto& n = tree.nodes()[id];

            n.set_aabb(node.min().v, node.max().v);

            if (0xFFFFFFFF != node.children[0]) {
                n.set_split_node(node.children[1], node.axis());
            } else {
                uint32_t i = node.start_index();
                uint8_t const num_primitives = node.num_indices();
                n.set_leaf_node(i, num_primitives);

                uint32_t const* const primitives = node.primitives;

                for (uint32_t p = 0,  len = uint32_t(num_primitives); p < len; ++p, ++i) {
                    auto const& t = triangles[primitives[p]];
                    tree.add_triangle(t.i[0], t.i[1], t.i[2], t.part, vertices, i);
                }
            }
        }
    },
    0, int32_t(build_nodes_.size()));
}

}  // namespace scene::shape::triangle::bvh

