#ifndef SU_CORE_SCENE_SHAPE_TRIANGLE_BVH_BUILDER_SAH_HPP
#define SU_CORE_SCENE_SHAPE_TRIANGLE_BVH_BUILDER_SAH_HPP

#include "scene/bvh/scene_bvh_builder_base.hpp"
#include "scene/bvh/scene_bvh_split_candidate.hpp"

#include <vector>

namespace thread {
class Pool;
}

namespace scene {

namespace bvh {
struct Reference;
class Node;
}  // namespace bvh

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
               uint32_t max_primitives, thread::Pool& threads);

  private:
    using Reference  = scene::bvh::Reference;
    using References = std::vector<Reference>;

    template <typename Data>
    void serialize(Build_node* node, Triangles triangles, Vertices vertices, Tree<Data>& tree,
                   uint32_t& current_triangle);

    void assign(Build_node* node, References const& references);
};

}  // namespace bvh
}  // namespace triangle
}  // namespace shape
}  // namespace scene

#endif
