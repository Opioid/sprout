#ifndef SU_CORE_SCENE_SHAPE_TRIANGLE_BVH_INDEXED_DATA_HPP
#define SU_CORE_SCENE_SHAPE_TRIANGLE_BVH_INDEXED_DATA_HPP

#include "base/math/ray.hpp"
#include "base/math/vector3.hpp"
#include "base/simd/simd.hpp"

namespace scene::shape {

struct Vertex;
class Vertex_stream;

namespace triangle {

struct Index_triangle;

namespace bvh {

struct Reference;

template <typename SV>
class Indexed_data {
  public:
    Indexed_data() noexcept;

    ~Indexed_data() noexcept;

    uint32_t num_triangles() const noexcept;

    bool intersect(uint32_t index, ray& ray, float2& uv) const noexcept;

    bool intersect_p(uint32_t index, ray const& ray) const noexcept;

    bool intersect(Simd3f const& origin, Simd3f const& direction, scalar const& min_t,
                   scalar& max_t, uint32_t index, scalar& u, scalar& v) const noexcept;

    bool intersect(Simd3f const& origin, Simd3f const& direction, scalar const& min_t,
                   scalar& max_t, uint32_t index) const noexcept;

    bool intersect_p(Simd3f const& origin, Simd3f const& direction, scalar const& min_t,
                     scalar const& max_t, uint32_t index) const noexcept;

    float3 interpolate_p(float2 uv, uint32_t index) const noexcept;

    Simd3f interpolate_p(Simd3f const& u, Simd3f const& v, uint32_t index) const noexcept;

    void interpolate_data(uint32_t index, float2 uv, float3& n, float3& t, float2& tc) const
        noexcept;

    void interpolate_data(Simd3f const& u, Simd3f const& v, uint32_t index, Simd3f& n, Simd3f& t,
                          float2& tc) const noexcept;

    Simd3f interpolate_shading_normal(Simd3f const& u, Simd3f const& v, uint32_t index) const
        noexcept;

    float2 interpolate_uv(uint32_t index, float2 uv) const noexcept;

    float2 interpolate_uv(Simd3f const& u, Simd3f const& v, uint32_t index) const noexcept;

    float bitangent_sign(uint32_t index) const noexcept;

    uint32_t material_index(uint32_t index) const noexcept;

    float3 normal(uint32_t index) const noexcept;

    Simd3f normal_v(uint32_t index) const noexcept;

    float area(uint32_t index) const noexcept;

    float area(uint32_t index, float3 const& scale) const noexcept;

    float3 center(uint32_t index) const noexcept;

    void sample(uint32_t index, float2 r2, float3& p, float2& tc) const noexcept;

    void allocate_triangles(uint32_t num_triangles, Vertex_stream const& vertices) noexcept;

    void add_triangle(uint32_t a, uint32_t b, uint32_t c, uint32_t material_index,
                      Vertex_stream const& vertices, uint32_t current_triangle) noexcept;

    size_t num_bytes() const noexcept;

    struct alignas(16) Index_triangle {
        Index_triangle(uint32_t a, uint32_t b, uint32_t c, uint8_t bitangent_sign,
                       uint32_t material_index) noexcept;

        uint32_t a, b, c;
        uint32_t bts : 1;
        uint32_t material_index : 31;
    };

  private:
    uint32_t num_triangles_;
    uint32_t num_vertices_;

    Index_triangle* triangles_;

    float3* intersection_vertices_;

    SV* shading_vertices_;
};

template <typename V>
class Indexed_data_interleaved {
  public:
    Indexed_data_interleaved();

    ~Indexed_data_interleaved();

    uint32_t num_triangles() const;

    uint32_t current_triangle() const;

    bool intersect(uint32_t index, ray& ray, float2& uv) const;

    bool intersect_p(uint32_t index, ray const& ray) const;

    void interpolate_data(uint32_t index, float2 uv, float3& n, float3& t, float2& tc) const;

    float2 interpolate_uv(uint32_t index, float2 uv) const;

    float bitangent_sign(uint32_t index) const;

    uint32_t material_index(uint32_t index) const;

    float3 normal(uint32_t index) const;

    float area(uint32_t index) const;

    float area(uint32_t index, float3 const& scale) const;

    void sample(uint32_t index, float2 r2, float3& p, float2& tc) const;

    void allocate_triangles(uint32_t num_triangles, uint32_t num_vertices, Vertex const* vertices);

    void add_triangle(uint32_t a, uint32_t b, uint32_t c, uint32_t material_index,
                      Vertex const* vertices);

    size_t num_bytes() const;

  private:
    uint32_t num_triangles_;
    uint32_t current_triangle_;
    uint32_t num_vertices_;

    struct alignas(16) Index_triangle {
        Index_triangle(uint32_t a, uint32_t b, uint32_t c, uint8_t bitangent_sign,
                       uint32_t material_index);

        uint32_t a, b, c;
        uint32_t bts_material_index;

        static uint32_t constexpr Material_index_mask = 0x7FFFFFFF;
        static uint32_t constexpr BTS_mask            = ~Material_index_mask;
    };

    Index_triangle* triangles_;

    V* vertices_;
};

}  // namespace bvh
}  // namespace triangle
}  // namespace scene::shape

#endif
