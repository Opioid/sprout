#ifndef SU_CORE_SCENE_SHAPE_TRIANGLE_BVH_TREE_HPP
#define SU_CORE_SCENE_SHAPE_TRIANGLE_BVH_TREE_HPP

#include "base/math/aabb.hpp"
#include "base/math/vector3.hpp"
#include "scene/material/sampler_settings.hpp"

#include <vector>

namespace math {
struct Ray;
}

namespace scene {

namespace material {
class Material;
}

namespace bvh {
class Node;
}

class Worker;

namespace shape {

class Node_stack;
struct Vertex;
class Vertex_stream;

namespace triangle {

struct Intersection;
struct Intersectioni;
struct Index_triangle;
struct Triangle;
struct Position_triangle;
struct Data_triangle;

namespace bvh {

template <typename Data>
class Tree {
  public:
    Tree();

    ~Tree();

    using Node      = scene::bvh::Node;
    using Filter    = material::Sampler_settings::Filter;
    using Material  = material::Material;
    using Materials = Material const* const*;

    Node* allocate_nodes(uint32_t num_nodes);

    AABB aabb() const;

    uint32_t num_parts() const;

    uint32_t num_triangles() const;
    uint32_t num_triangles(uint32_t part) const;

    //    bool intersect(ray& ray, Node_stack& node_stack, Intersection& intersection) const;

    //    bool intersect(ray& ray, Node_stack& node_stack) const;

    bool intersect(Simd3f const& ray_origin, Simd3f const& ray_direction, scalar const& ray_min_t,
                   scalar& ray_max_t, Node_stack& node_stack, Intersection& intersection) const;

    bool intersect(Simd3f const& ray_origin, Simd3f const& ray_direction, scalar const& ray_min_t,
                   scalar& ray_max_t, Node_stack& node_stack) const;

    //    bool intersect_p(ray const& ray, Node_stack& node_stack) const;

    bool intersect_p(Simd3f const& ray_origin, Simd3f const& ray_direction, scalar const& ray_min_t,
                     scalar const& ray_max_t, Node_stack& node_stack) const;

    float opacity(ray& ray, uint64_t time, uint32_t entity, Filter filter,
                  Worker const& worker) const;

    bool absorption(ray& ray, uint64_t time, uint32_t entity, Filter filter, Worker const& worker,
                    float3& ta) const;

    float3 interpolate_p(float2 uv, uint32_t index) const;

    Simd3f interpolate_p(Simd3f const& u, Simd3f const& v, uint32_t index) const;

    void interpolate_triangle_data(uint32_t index, float2 uv, float3& n, float3& t,
                                   float2& tc) const;

    void interpolate_triangle_data(Simd3f const& u, Simd3f const& v, uint32_t index, Simd3f& n,
                                   Simd3f& t, float2& tc) const;

    Simd3f interpolate_shading_normal(Simd3f const& u, Simd3f const& v, uint32_t index) const;

    float2 interpolate_triangle_uv(uint32_t index, float2 uv) const;

    float2 interpolate_triangle_uv(Simd3f const& u, Simd3f const& v, uint32_t index) const;

    float triangle_bitangent_sign(uint32_t index) const;

    uint32_t triangle_part(uint32_t index) const;

    float3 triangle_normal(uint32_t index) const;

    Simd3f triangle_normal_v(uint32_t index) const;

    float triangle_area(uint32_t index) const;

    float triangle_area(uint32_t index, float3 const& scale) const;

    float3 triangle_center(uint32_t index) const;

    //    void sample(uint32_t index, float2 r2, float3& p, float3& n, float2& tc) const ;
    void sample(uint32_t index, float2 r2, float3& p, float2& tc) const;
    //    void sample(uint32_t index, float2 r2, float3& p) const ;

    void allocate_parts(uint32_t num_parts);

    void allocate_triangles(uint32_t num_triangles, Vertex_stream const& vertices);

    void add_triangle(uint32_t a, uint32_t b, uint32_t c, uint32_t material_index,
                      Vertex_stream const& vertices, uint32_t current_triangle);

    size_t num_bytes() const;

  private:
    uint32_t num_nodes_;
    uint32_t num_parts_;

    Node*     nodes_;
    uint32_t* num_part_triangles_;

    Data data_;
};

}  // namespace bvh
}  // namespace triangle
}  // namespace shape
}  // namespace scene

#endif
