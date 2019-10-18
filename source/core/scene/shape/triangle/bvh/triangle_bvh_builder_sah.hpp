#ifndef SU_CORE_SCENE_SHAPE_TRIANGLE_BVH_BUILDER_SAH_HPP
#define SU_CORE_SCENE_SHAPE_TRIANGLE_BVH_BUILDER_SAH_HPP

#include "scene/bvh/scene_bvh_builder_base.hpp"
#include "base/math/aabb.hpp"
#include "base/math/vector3.hpp"
#include "scene/bvh/scene_bvh_split_candidate.hpp"

#include <vector>

namespace thread {
class Pool;
}

namespace scene {

namespace bvh {
struct Reference;
class Node;
}

namespace shape {

class Vertex_stream;

namespace triangle {

struct Index_triangle;

namespace bvh {

template <typename Data>
class Tree;

class Builder_SAH : private scene::bvh::Builder_base {
  public:
    Builder_SAH(uint32_t num_slices, uint32_t sweep_threshold);

    using Triangles = Index_triangle const* const;
    using Vertices  = Vertex_stream const&;

    template <typename Data>
    void build(Tree<Data>& tree, uint32_t num_triangles, Triangles triangles, Vertices vertices,
               uint32_t max_primitives, thread::Pool& thread_pool);

  private:
    using Reference = scene::bvh::Reference;
    using References = std::vector<Reference>;

    void split(Build_node* node, References& references, AABB const& aabb, uint32_t max_primitives,
               uint32_t depth, thread::Pool& thread_pool);

    using Split_candidate = scene::bvh::Split_candidate;

    Split_candidate splitting_plane(References const& references, AABB const& aabb, uint32_t depth,
                                    bool& exhausted, thread::Pool& thread_pool);

    template <typename Data>
    void serialize(Build_node* node, Triangles triangles, Vertices vertices, Tree<Data>& tree,
                   uint32_t& current_triangle);

    using Node = scene::bvh::Node;

    Node& new_node();

    uint32_t current_node_index() const;

    void assign(Build_node* node, References const& references);

    std::vector<Split_candidate> split_candidates_;

    uint32_t num_nodes_;
    uint32_t current_node_;

    Node* nodes_;

    uint32_t num_references_;

    uint32_t spatial_split_threshold_;

    uint32_t const num_slices_;
    uint32_t const sweep_threshold_;
};

}  // namespace bvh
}  // namespace triangle
}  // namespace shape
}  // namespace scene

#endif
