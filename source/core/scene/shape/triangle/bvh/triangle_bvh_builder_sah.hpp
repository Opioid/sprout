#ifndef SU_CORE_SCENE_SHAPE_TRIANGLE_BVH_BUILDER_SAH_HPP
#define SU_CORE_SCENE_SHAPE_TRIANGLE_BVH_BUILDER_SAH_HPP

//#include "triangle_bvh_builder_base.hpp"

#include "base/math/aabb.hpp"
#include "base/math/plane.hpp"
#include "base/math/vector3.hpp"

#include <vector>

namespace thread {
class Pool;
}

namespace scene {

namespace bvh {
class Node;
}

namespace shape {

class Vertex_stream;

namespace triangle {

struct Index_triangle;

namespace bvh {

struct Reference {
    Reference() {}

    uint32_t primitive() const;

    void set_min_max_primitive(Simd3f const& min, Simd3f const& max, uint32_t primitive);

    void clip_min(float d, uint8_t axis);
    void clip_max(float d, uint8_t axis);

    struct alignas(16) Vector {
        float    v[3];
        uint32_t index;
    };

    Vector bounds[2];
};

template <typename Data>
class Tree;

class Builder_SAH /*: private Builder_base*/ {
  public:
    Builder_SAH(uint32_t num_slices, uint32_t sweep_threshold);

    using Triangles = Index_triangle const* const;
    using Vertices  = Vertex_stream const&;

    template <typename Data>
    void build(Tree<Data>& tree, uint32_t num_triangles, Triangles triangles, Vertices vertices,
               uint32_t max_primitives, thread::Pool& thread_pool);

  private:
    using References = std::vector<Reference>;

    class Split_candidate {
      public:
        Split_candidate(uint8_t split_axis, float3 const& p, bool spatial);

        void evaluate(References const& references, float aabb_surface_area);

        void distribute(References const& references, References& references0,
                        References& references1) const;

        float cost() const;

        bool behind(float const* point) const;

        uint8_t axis() const;

        bool spatial() const;

        AABB const& aabb_0() const;
        AABB const& aabb_1() const;

        uint32_t num_side_0() const;
        uint32_t num_side_1() const;

      private:
        AABB aabb_0_;
        AABB aabb_1_;

        uint32_t num_side_0_;
        uint32_t num_side_1_;

        float d_;

        float cost_;

        uint8_t axis_;

        bool spatial_;
    };

    struct Build_node {
        Build_node();
        ~Build_node();

        AABB aabb;

        std::vector<uint32_t> primitives;

        uint32_t start_index;
        uint32_t end_index;

        uint8_t axis;

        Build_node* children[2];
    };

    void split(Build_node* node, References& references, AABB const& aabb, uint32_t max_primitives,
               uint32_t depth, thread::Pool& thread_pool);

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
