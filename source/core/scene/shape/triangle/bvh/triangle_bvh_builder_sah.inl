#ifndef SU_CORE_SCENE_SHAPE_TRIANGLE_BVH_BUILDER_SAH_INL
#define SU_CORE_SCENE_SHAPE_TRIANGLE_BVH_BUILDER_SAH_INL

#include "triangle_bvh_builder_sah.hpp"
//#include "triangle_bvh_builder_base.inl"
#include "base/math/aabb.inl"
#include "base/math/vector3.inl"
#include "base/memory/array.inl"
#include "base/thread/thread_pool.hpp"
#include "scene/bvh/scene_bvh_node.inl"
#include "scene/bvh/scene_bvh_split_candidate.inl"
#include "scene/shape/node_stack.hpp"
#include "scene/shape/shape_vertex.hpp"
#include "scene/shape/triangle/triangle_primitive.hpp"
#include "triangle_bvh_helper.hpp"
#include "triangle_bvh_tree.inl"

#include <vector>

namespace scene::shape::triangle::bvh {

template <typename Data>
void Builder_SAH::build(Tree<Data>& tree, uint32_t num_triangles, Triangles triangles,
                        Vertices vertices, uint32_t max_primitives, thread::Pool& threads) {
    Build_node root;

    {
        float const log2_num_triangles = std::log2(float(num_triangles));

        spatial_split_threshold_ = uint32_t(log2_num_triangles / 2.f + 0.5f);

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

                    references[i].set(min, max, uint32_t(i));

                    aabb.merge_assign(min, max);
                }

                aabbs[id] = aabb;
                // aabbs[id].merge_assign(aabb);
            },
            0, num_triangles);

        Simd_AABB aabb(AABB::empty());
        for (auto& b : aabbs) {
            aabb.merge_assign(b);
        }

        num_nodes_      = 1;
        num_references_ = 0;

        split(&root, references, AABB(aabb.min, aabb.max), max_primitives, 0, threads);
    }

    tree.allocate_triangles(num_references_, vertices);

    nodes_ = tree.allocate_nodes(num_nodes_);

    current_node_ = 0;

    uint32_t current_triangle = 0;
    serialize(&root, triangles, vertices, tree, current_triangle);
}

template <typename Data>
void Builder_SAH::serialize(Build_node* node, Triangles triangles, Vertices vertices,
                            Tree<Data>& tree, uint32_t& current_triangle) {
    auto& n = new_node();
    n.set_aabb(node->aabb.min().v, node->aabb.max().v);

    if (node->children[0]) {
        serialize(node->children[0], triangles, vertices, tree, current_triangle);

        n.set_split_node(current_node_index(), node->axis);

        serialize(node->children[1], triangles, vertices, tree, current_triangle);
    } else {
        uint8_t const num_primitives = uint8_t(node->end_index - node->start_index);
        n.set_leaf_node(node->start_index, num_primitives);

        uint32_t i = current_triangle;
        for (auto const p : node->primitives) {
            auto const& t = triangles[p];
            tree.add_triangle(t.i[0], t.i[1], t.i[2], t.part, vertices, i);
            ++i;
        }

        current_triangle = i;
    }
}

}  // namespace scene::shape::triangle::bvh

#endif
