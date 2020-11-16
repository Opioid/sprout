#ifndef SU_CORE_SCENE_SHAPE_TRIANGLE_BVH_DATA_HPP
#define SU_CORE_SCENE_SHAPE_TRIANGLE_BVH_DATA_HPP

#include "base/math/ray.hpp"
#include "base/math/vector3.hpp"

namespace scene::shape {

struct Vertex;

namespace triangle::bvh {

template <typename Intersection_triangle, typename Shading_triangle>
class Data {
  public:
    Data();

    ~Data();

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
    float area(uint32_t index, float3_p scale) const;

    //	void sample(uint32_t index, float2 r2, float3& p, float3& n, float2& tc) const;
    void sample(uint32_t index, float2 r2, float3& p, float2& tc) const;
    //	void sample(uint32_t index, float2 r2, float3& p) const;

    void allocate_triangles(uint32_t num_triangles, uint32_t num_vertices,
                            Vertex const* const vertices);

    void add_triangle(uint32_t a, uint32_t b, uint32_t c, uint32_t material_index,
                      Vertex const* const vertices);

  private:
    uint32_t num_triangles_;
    uint32_t current_triangle_;

    Intersection_triangle* intersection_triangles_;
    Shading_triangle*      shading_triangles_;
};

}  // namespace triangle::bvh
}  // namespace scene::shape

#endif
