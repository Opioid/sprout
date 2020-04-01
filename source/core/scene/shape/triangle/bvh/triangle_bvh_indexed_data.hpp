#ifndef SU_CORE_SCENE_SHAPE_TRIANGLE_BVH_INDEXED_DATA_HPP
#define SU_CORE_SCENE_SHAPE_TRIANGLE_BVH_INDEXED_DATA_HPP

#include "base/math/ray.hpp"
#include "base/math/vector3.hpp"
#include "base/simd/simd.hpp"

namespace scene::shape {

class Vertex_stream;

namespace triangle::bvh {

template <typename SV>
class Indexed_data {
  public:
    Indexed_data();

    ~Indexed_data();

    uint32_t num_triangles() const;

    bool intersect(uint32_t index, ray& ray, float2& uv) const;

    bool intersect_p(uint32_t index, ray const& ray) const;

    bool intersect(Simd3f const& origin, Simd3f const& direction, scalar const& min_t,
                   scalar& max_t, uint32_t index, scalar& u, scalar& v) const;

    bool intersect(Simd3f const& origin, Simd3f const& direction, scalar const& min_t,
                   scalar& max_t, uint32_t index) const;

    bool intersect_p(Simd3f const& origin, Simd3f const& direction, scalar const& min_t,
                     scalar const& max_t, uint32_t index) const;

    float3 interpolate_p(float2 uv, uint32_t index) const;

    Simd3f interpolate_p(Simd3f const& u, Simd3f const& v, uint32_t index) const;

    void interpolate_data(uint32_t index, float2 uv, float3& n, float3& t, float2& tc) const;

    void interpolate_data(Simd3f const& u, Simd3f const& v, uint32_t index, Simd3f& n, Simd3f& t,
                          float2& tc) const;

    Simd3f interpolate_shading_normal(Simd3f const& u, Simd3f const& v, uint32_t index) const;

    float2 interpolate_uv(uint32_t index, float2 uv) const;

    float2 interpolate_uv(Simd3f const& u, Simd3f const& v, uint32_t index) const;

    float bitangent_sign(uint32_t index) const;

    uint32_t part(uint32_t index) const;

    float3 normal(uint32_t index) const;

    Simd3f normal_v(uint32_t index) const;

    float area(uint32_t index) const;

    float area(uint32_t index, float3 const& scale) const;

    float3 center(uint32_t index) const;

    void sample(uint32_t index, float2 r2, float3& p, float2& tc) const;

    void allocate_triangles(uint32_t num_triangles, Vertex_stream const& vertices);

    void add_triangle(uint32_t a, uint32_t b, uint32_t c, uint32_t part,
                      Vertex_stream const& vertices, uint32_t current_triangle);

    size_t num_bytes() const;

    struct alignas(16) Index_triangle {
        Index_triangle(uint32_t a, uint32_t b, uint32_t c, uint8_t bitangent_sign, uint32_t part);

        uint32_t a, b, c;
        uint32_t bts : 1;
        uint32_t part : 31;
    };

  private:
    uint32_t num_triangles_;
    uint32_t num_vertices_;

    Index_triangle* triangles_;

    float3* intersection_vertices_;

    SV* shading_vertices_;
};

class Indexed_data1 {
  public:
    Indexed_data1();

    ~Indexed_data1();

    uint32_t num_triangles() const;

    bool intersect(uint32_t index, ray& ray, float2& uv) const;

    bool intersect_p(uint32_t index, ray const& ray) const;

    bool intersect(Simd3f const& origin, Simd3f const& direction, scalar const& min_t,
                   scalar& max_t, uint32_t index, scalar& u, scalar& v) const;

    bool intersect(Simd3f const& origin, Simd3f const& direction, scalar const& min_t,
                   scalar& max_t, uint32_t index) const;

    bool intersect_p(Simd3f const& origin, Simd3f const& direction, scalar const& min_t,
                     scalar const& max_t, uint32_t index) const;

    float3 interpolate_p(float2 uv, uint32_t index) const;

    Simd3f interpolate_p(Simd3f const& u, Simd3f const& v, uint32_t index) const;

    void interpolate_data(uint32_t index, float2 uv, float3& n, float3& t, float2& tc) const;

    void interpolate_data(Simd3f const& u, Simd3f const& v, uint32_t index, Simd3f& n, Simd3f& t,
                          float2& tc) const;

    void interpolate_data(Simd3f const& u, Simd3f const& v, uint32_t index, Simd3f& t, Simd3f& b, Simd3f& n,
                          float2& tc) const;

    Simd3f interpolate_shading_normal(Simd3f const& u, Simd3f const& v, uint32_t index) const;

    float2 interpolate_uv(uint32_t index, float2 uv) const;

    float2 interpolate_uv(Simd3f const& u, Simd3f const& v, uint32_t index) const;

    float bitangent_sign(uint32_t index) const;

    uint32_t part(uint32_t index) const;

    float3 normal(uint32_t index) const;

    Simd3f normal_v(uint32_t index) const;

    float area(uint32_t index) const;

    float area(uint32_t index, float3 const& scale) const;

    float3 center(uint32_t index) const;

    void sample(uint32_t index, float2 r2, float3& p, float2& tc) const;

    void allocate_triangles(uint32_t num_triangles, Vertex_stream const& vertices);

    void add_triangle(uint32_t a, uint32_t b, uint32_t c, uint32_t part,
                      Vertex_stream const& vertices, uint32_t current_triangle);

    size_t num_bytes() const;

    struct alignas(16) Index_triangle {
        Index_triangle(uint32_t a, uint32_t b, uint32_t c, uint8_t bitangent_sign, uint32_t part);

        uint32_t a, b, c;
        uint32_t bts : 1;
        uint32_t part : 31;
    };

  private:
    uint32_t num_triangles_;
    uint32_t num_vertices_;

    Index_triangle* triangles_;

    float3* intersection_vertices_;

    float4* tangent_frames_;

    float2* uvs_;
};

}  // namespace bvh
}  // namespace scene::shape

#endif
