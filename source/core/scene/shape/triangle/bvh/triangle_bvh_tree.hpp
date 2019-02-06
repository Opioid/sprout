#ifndef SU_CORE_SCENE_SHAPE_TRIANGLE_BVH_TREE_HPP
#define SU_CORE_SCENE_SHAPE_TRIANGLE_BVH_TREE_HPP

#include <vector>
#include "base/math/aabb.hpp"
#include "base/math/vector3.hpp"
#include "scene/material/sampler_settings.hpp"

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

namespace triangle {

struct Intersection;
struct Index_triangle;
struct Triangle;
struct Position_triangle;
struct Data_triangle;

namespace bvh {

template <typename Data>
class Tree {
  public:
    Tree() noexcept;

    ~Tree() noexcept;

    using Node      = scene::bvh::Node;
    using Filter    = material::Sampler_settings::Filter;
    using Material  = material::Material;
    using Materials = Material const* const*;

    Node* allocate_nodes(uint32_t num_nodes) noexcept;

    AABB aabb() const noexcept;

    uint32_t num_parts() const noexcept;

    uint32_t num_triangles() const noexcept;
    uint32_t num_triangles(uint32_t part) const noexcept;

    uint32_t current_triangle() const noexcept;

    bool intersect(ray& ray, Node_stack& node_stack, Intersection& intersection) const noexcept;

    bool intersect(ray& ray, Node_stack& node_stack) const noexcept;

    bool intersect(FVector ray_origin, FVector ray_direction, FVector ray_inv_direction,
                   FVector ray_min_t, Vector& ray_max_t, uint32_t ray_signs[4],
                   Node_stack& node_stack, Intersection& intersection) const noexcept;

    bool intersect(FVector ray_origin, FVector ray_direction, FVector ray_inv_direction,
                   FVector ray_min_t, Vector& ray_max_t, uint32_t ray_signs[4],
                   Node_stack& node_stack) const noexcept;

    bool intersect_p(ray const& ray, Node_stack& node_stack) const noexcept;

    bool intersect_p(FVector ray_origin, FVector ray_direction, FVector ray_inv_direction,
                     FVector ray_min_t, FVector ray_max_t, uint32_t ray_signs[4],
                     Node_stack& node_stack) const noexcept;

    float opacity(ray& ray, uint64_t time, Materials materials, Filter filter,
                  Worker const& worker) const noexcept;

    float3 absorption(ray& ray, uint64_t time, Materials materials, Filter filter,
                      Worker const& worker) const noexcept;

    void interpolate_triangle_data(uint32_t index, float2 uv, float3& n, float3& t,
                                   float2& tc) const noexcept;

    void interpolate_triangle_data(FVector u, FVector v, uint32_t index, float3& n, float3& t,
                                   float2& tc) const noexcept;

    void interpolate_triangle_data(FVector u, FVector v, uint32_t index, Vector& n, Vector& t,
                                   float2& tc) const noexcept;

    float2 interpolate_triangle_uv(uint32_t index, float2 uv) const noexcept;

    float2 interpolate_triangle_uv(FVector u, FVector v, uint32_t index) const noexcept;

    float triangle_bitangent_sign(uint32_t index) const noexcept;

    uint32_t triangle_material_index(uint32_t index) const noexcept;

    float3 triangle_normal(uint32_t index) const noexcept;
    Vector triangle_normal_v(uint32_t index) const noexcept;

    float triangle_area(uint32_t index) const noexcept;
    float triangle_area(uint32_t index, float3 const& scale) const noexcept;

    //    void sample(uint32_t index, float2 r2, float3& p, float3& n, float2& tc) const noexcept;
    void sample(uint32_t index, float2 r2, float3& p, float2& tc) const noexcept;
    //    void sample(uint32_t index, float2 r2, float3& p) const noexcept;

    void allocate_parts(uint32_t num_parts) noexcept;

    void allocate_triangles(uint32_t num_triangles, uint32_t num_vertices,
                            Vertex const* const vertices) noexcept;

    void add_triangle(uint32_t a, uint32_t b, uint32_t c, uint32_t material_index,
                      Vertex const* const vertices) noexcept;

    size_t num_bytes() const noexcept;

  private:
    uint32_t num_nodes_;
    Node*    nodes_;

    uint32_t  num_parts_;
    uint32_t* num_part_triangles_;

    Data data_;
};

}  // namespace bvh
}  // namespace triangle
}  // namespace shape
}  // namespace scene

#endif
