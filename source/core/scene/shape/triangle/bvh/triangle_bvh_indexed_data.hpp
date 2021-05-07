#ifndef SU_CORE_SCENE_SHAPE_TRIANGLE_BVH_INDEXED_DATA_HPP
#define SU_CORE_SCENE_SHAPE_TRIANGLE_BVH_INDEXED_DATA_HPP

#include "base/math/ray.hpp"
#include "base/math/vector3.hpp"
#include "base/simd/simd.hpp"
#include "scene/shape/triangle/triangle_primitive_mt.hpp"

namespace scene::shape {

class Vertex_stream;

namespace triangle::bvh {

class Indexed_data {
  public:
    Indexed_data();

    ~Indexed_data();

    uint32_t num_triangles() const;

    bool intersect(uint32_t index, ray& ray, float2& uv) const;

    bool intersect_p(uint32_t index, ray const& ray) const;

    bool intersect(Simd3f_p origin, Simd3f_p direction, scalar_p min_t, scalar& max_t,
                   uint32_t index, scalar& u, scalar& v) const;

    bool intersect(Simd3f_p origin, Simd3f_p direction, scalar_p min_t, scalar& max_t,
                   uint32_t index) const;

    bool intersect_p(Simd3f_p origin, Simd3f_p direction, scalar_p min_t, scalar_p max_t,
                     uint32_t index) const;

    float3 interpolate_p(float2 uv, uint32_t index) const;

    Simd3f interpolate_p(Simd3f_p u, Simd3f_p v, uint32_t index) const;

    void interpolate_data(Simd3f_p u, Simd3f_p v, uint32_t index, Simd3f& n, Simd3f& t,
                          float2& tc) const;

    Simd3f interpolate_shading_normal(Simd3f_p u, Simd3f_p v, uint32_t index) const;

    float2 interpolate_uv(uint32_t index, float2 uv) const;

    float2 interpolate_uv(Simd3f_p u, Simd3f_p v, uint32_t index) const;

    float bitangent_sign(uint32_t index) const;

    uint32_t part(uint32_t index) const;

    float3 normal(uint32_t index) const;

    Simd3f normal_v(uint32_t index) const;

    float area(uint32_t index) const;

    float3 center(uint32_t index) const;

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
}  // namespace scene::shape

#endif
