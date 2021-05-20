#ifndef SU_CORE_SCENE_SHAPE_TRIANGLE_BVH_TREE_HPP
#define SU_CORE_SCENE_SHAPE_TRIANGLE_BVH_TREE_HPP

#include "base/math/aabb.hpp"
#include "base/math/vector3.hpp"
#include "scene/material/sampler_settings.hpp"
#include "triangle_bvh_indexed_data.hpp"

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

struct Index_triangle;
struct Triangle;
struct Position_triangle;
struct Data_triangle;

struct Intersection {
    Simdf u;
    Simdf v;

    uint32_t index;
};

namespace bvh {

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

    bool intersect(Simdf_p ray_origin, Simdf_p ray_direction, scalar_p ray_min_t, scalar& ray_max_t,
                   Node_stack& nodes, Intersection& isec) const;

    bool intersect(Simdf_p ray_origin, Simdf_p ray_direction, scalar_p ray_min_t, scalar& ray_max_t,
                   Node_stack& nodes) const;

    bool intersect_p(Simdf_p ray_origin, Simdf_p ray_direction, scalar_p ray_min_t,
                     scalar_p ray_max_t, Node_stack& nodes) const;

    float visibility(ray& ray, uint32_t entity, Filter filter, Worker& worker) const;

    bool absorption(ray& ray, uint32_t entity, Filter filter, Worker& worker, float3& ta) const;

    float3 interpolate_p(float2 uv, uint32_t index) const;

    Simdf interpolate_p(Simdf_p u, Simdf_p v, uint32_t index) const;

    void interpolate_triangle_data(Simdf_p u, Simdf_p v, uint32_t index, Simdf& n, Simdf& t,
                                   float2& tc) const;

    Simdf interpolate_shading_normal(Simdf_p u, Simdf_p v, uint32_t index) const;

    float2 interpolate_triangle_uv(Simdf_p u, Simdf_p v, uint32_t index) const;

    float triangle_bitangent_sign(uint32_t index) const;

    uint32_t triangle_part(uint32_t index) const;

    float3 triangle_normal(uint32_t index) const;

    Simdf triangle_normal_v(uint32_t index) const;

    float triangle_area(uint32_t index) const;

    void triangle(uint32_t index, float3& pa, float3& pb, float3& pc) const;

    void triangle(uint32_t index, float3& pa, float3& pb, float3& pc, float2& uva, float2& uvb,
                  float2& uvc) const;

    void sample(uint32_t index, float2 r2, float3& p, float2& tc) const;

    void allocate_parts(uint32_t num_parts);

    void allocate_triangles(uint32_t num_triangles, Vertex_stream const& vertices);

    void set_triangle(uint32_t a, uint32_t b, uint32_t c, uint32_t material_index,
                      Vertex_stream const& vertices, uint32_t triangle_id);

  private:
    uint32_t num_nodes_;
    uint32_t num_parts_;

    Node* nodes_;

    Indexed_data data_;
};

}  // namespace bvh
}  // namespace triangle
}  // namespace shape
}  // namespace scene

#endif
