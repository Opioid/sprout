#ifndef SU_CORE_SCENE_SHAPE_TRIANGLE_BVH_BUILDER_SAH_HPP
#define SU_CORE_SCENE_SHAPE_TRIANGLE_BVH_BUILDER_SAH_HPP

#include "scene/bvh/scene_bvh_builder_base.hpp"
#include "scene/bvh/scene_bvh_split_candidate.hpp"
#include "triangle_bvh_tree.hpp"

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

class Builder_SAH final : private scene::bvh::Builder_base {
  public:
    Builder_SAH(uint32_t num_slices, uint32_t sweep_threshold, uint32_t max_primitives);

    ~Builder_SAH();

    using Triangles = Index_triangle const* const;
    using Vertices  = Vertex_stream const&;

    void build(Tree& tree, uint32_t num_triangles, Triangles triangles, Vertices vertices,
               Threads& threads);

    void build(Tree& tree, uint32_t num_triangles, Triangles triangles, Vertices vertices,
               uint32_t num_frames, Threads& threads);

  private:
    using Node       = scene::bvh::Node;
    using Reference  = scene::bvh::Reference;
    using References = scene::bvh::References;

    void serialize(uint32_t source_node, uint32_t dest_node, Triangles triangles, Vertices vertices,
                   Tree& tree, uint32_t& current_triangle);
};

}  // namespace bvh
}  // namespace triangle
}  // namespace shape
}  // namespace scene

#endif
