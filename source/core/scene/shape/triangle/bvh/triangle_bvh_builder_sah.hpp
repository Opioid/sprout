#ifndef SU_CORE_SCENE_SHAPE_TRIANGLE_BVH_BUILDER_SAH_HPP
#define SU_CORE_SCENE_SHAPE_TRIANGLE_BVH_BUILDER_SAH_HPP

#include "scene/bvh/scene_bvh_builder_base.hpp"
#include "scene/bvh/scene_bvh_split_candidate.hpp"
#include "scene/shape/triangle/triangle_mesh_bvh.hpp"

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

    ~Builder_SAH() final;

    using Triangles = Index_triangle const* const;
    using Vertices  = Vertex_stream const&;

    void build(triangle::Tree& tree, uint32_t num_triangles, Triangles triangles, Vertices vertices,
               thread::Pool& threads);

  private:
    using Reference  = scene::bvh::Reference;
    using References = std::vector<Reference>;

    void serialize(Triangles triangles, Vertices vertices, triangle::Tree& tree, thread::Pool& threads) const;
};

}  // namespace bvh
}  // namespace triangle
}  // namespace shape
}  // namespace scene

#endif
