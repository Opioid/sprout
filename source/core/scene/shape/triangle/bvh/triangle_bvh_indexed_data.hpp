#ifndef SU_CORE_SCENE_SHAPE_TRIANGLE_BVH_INDEXED_DATA_HPP
#define SU_CORE_SCENE_SHAPE_TRIANGLE_BVH_INDEXED_DATA_HPP

#include "base/math/ray.hpp"
#include "base/math/simd.hpp"
#include "base/math/vector3.hpp"
#include "scene/material/sampler_settings.hpp"
#include "scene/shape/triangle/triangle_primitive_mt.hpp"

namespace scene {

class Worker;

namespace shape {

class Vertex_stream;

namespace triangle::bvh {

class Indexed_data {
  public:
    using Filter = material::Sampler_settings::Filter;

    Indexed_data();

    ~Indexed_data();

    uint32_t num_triangles() const;

    bool intersect(SimdVec origin, SimdVec direction, Simdf min_t, Simdf& max_t, uint32_t begin,
                   uint32_t end, Simdf& u, Simdf& v, uint32_t& index) const;

    bool visibility(SimdVec origin, SimdVec direction, Simdf min_t, Simdf max_t, uint32_t begin,
                    uint32_t end, float3_p ray_dir, uint32_t entity, Filter filter, Worker& worker,
                    float3& vis) const;

    bool intersect_p(SimdVec origin, SimdVec direction, Simdf min_t, Simdf max_t, uint32_t begin,
                     uint32_t end) const;

    Simdf interpolate_p(Simdf_p u, Simdf_p v, uint32_t index) const;

    void interpolate_data(Simdf_p u, Simdf_p v, uint32_t index, Simdf& n, Simdf& t,
                          float2& tc) const;

    Simdf interpolate_shading_normal(Simdf_p u, Simdf_p v, uint32_t index) const;

    float2 interpolate_uv(Simdf_p u, Simdf_p v, uint32_t index) const;

    float bitangent_sign(uint32_t index) const;

    uint32_t part(uint32_t index) const;

    Simdf normal(uint32_t index) const;

    float area(uint32_t index) const;

    void triangle(uint32_t index, float3& pa, float3& pb, float3& pc) const;

    void triangle(uint32_t index, float3& pa, float3& pb, float3& pc, float2& uva, float2& uvb,
                  float2& uvc) const;

    void sample(uint32_t index, float2 r2, float3& p, float2& tc) const;

    void allocate_triangles(uint32_t num_triangles, Vertex_stream const& vertices);

    void set_triangle(uint32_t a, uint32_t b, uint32_t c, uint32_t part,
                      Vertex_stream const& vertices, uint32_t triangle_id);

    struct alignas(16) Index_triangle {
        Index_triangle();

        Index_triangle(uint32_t a, uint32_t b, uint32_t c, bool bitangent_sign, uint32_t part);

        uint32_t a, b, c;
        uint32_t bts : 1;
        uint32_t part : 31;
    };

  private:
    uint32_t num_triangles_;
    uint32_t num_vertices_;

    Index_triangle* triangles_;

    float3* positions_;

    using SV = shape::triangle::Shading_vertex_MTC;

    SV* shading_vertices_;
};

}  // namespace triangle::bvh
}  // namespace shape
}  // namespace scene

#endif
