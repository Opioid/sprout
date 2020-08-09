#include "triangle_bvh_builder_sah.hpp"
#include "base/math/aabb.inl"
#include "base/math/vector3.inl"
#include "base/memory/array.inl"
#include "base/thread/thread_pool.hpp"
#include "logging/logging.hpp"
#include "scene/bvh/scene_bvh_node.inl"
#include "scene/bvh/scene_bvh_split_candidate.inl"
#include "scene/shape/shape_vertex.hpp"
#include "scene/shape/triangle/triangle_primitive.hpp"
#include "triangle_bvh_helper.hpp"

namespace scene::shape::triangle::bvh {

Builder_SAH::Builder_SAH(uint32_t num_slices, uint32_t sweep_threshold, uint32_t max_primitives)
    : Builder_base(num_slices, sweep_threshold, max_primitives) {}

Builder_SAH::~Builder_SAH() = default;

void Builder_SAH::build(Tree& tree, uint32_t num_triangles, Triangles triangles, Vertices vertices,
                        thread::Pool& threads) {
    reserve(num_triangles);

    {
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

        split(references, AABB(aabb.min, aabb.max), threads);
    }

    tree.allocate_triangles(uint32_t(reference_ids_.size()), vertices);
    nodes_ = tree.allocate_nodes(uint32_t(build_nodes_.size()));

    uint32_t current_triangle = 0;
    new_node();
    serialize(0, 0, triangles, vertices, tree, current_triangle);
}

void Builder_SAH::serialize(uint32_t source_node, uint32_t dest_node, Triangles triangles,
                            Vertices vertices, Tree& tree, uint32_t& current_triangle) {
    Node const& node = build_nodes_[source_node];

    auto& n = nodes_[dest_node];

    n.set_aabb(node.min().v, node.max().v);

    if (0 == node.num_indices()) {
        uint32_t const child0 = current_node_index();

        n.set_split_node(child0, node.axis());

        new_node();
        new_node();

        uint32_t const source_child0 = node.children();

        serialize(source_child0, child0, triangles, vertices, tree, current_triangle);

        serialize(source_child0 + 1, child0 + 1, triangles, vertices, tree, current_triangle);
    } else {
        uint32_t      i   = current_triangle;
        uint8_t const num = node.num_indices();
        n.set_leaf_node(i, num);

        uint32_t const ro = node.children();

        for (uint32_t p = ro, end = ro + uint32_t(num); p < end; ++p, ++i) {
            auto const& t = triangles[reference_ids_[p]];
            tree.set_triangle(t.i[0], t.i[1], t.i[2], t.part, vertices, i);
        }

        current_triangle = i;
    }
}

}  // namespace scene::shape::triangle::bvh
