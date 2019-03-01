#ifndef SU_CORE_SCENE_SHAPE_TRIANGLE_BVH_DATA_INTERLEAVED_INL
#define SU_CORE_SCENE_SHAPE_TRIANGLE_BVH_DATA_INTERLEAVED_INL

#include "base/math/sampling.inl"
#include "scene/shape/triangle/triangle_primitive_mt.inl"
#include "triangle_bvh_data_interleaved.hpp"

namespace scene::shape::triangle::bvh {

template <typename Triangle>
Data_interleaved<Triangle>::~Data_interleaved() {}

template <typename Triangle>
uint32_t Data_interleaved<Triangle>::num_triangles() const {
    return static_cast<uint32_t>(triangles_.capacity());
}

template <typename Triangle>
uint32_t Data_interleaved<Triangle>::current_triangle() const {
    return static_cast<uint32_t>(triangles_.size());
}

template <typename Triangle>
bool Data_interleaved<Triangle>::intersect(uint32_t index, ray& ray, float2& uv) const {
    return triangles_[index].intersect(ray, uv);
}

template <typename Triangle>
bool Data_interleaved<Triangle>::intersect_p(uint32_t index, ray const& ray) const {
    return triangles_[index].intersect_p(ray);
}

template <typename Triangle>
bool Data_interleaved<Triangle>::intersect_p(FVector /*origin*/, FVector /*direction*/,
                                             FVector /*min_t*/, FVector /*max_t*/,
                                             uint32_t /*index*/) const {
    return false;
}

template <typename Triangle>
void Data_interleaved<Triangle>::interpolate_data(uint32_t index, float2 uv, float3& n, float3& t,
                                                  float2& tc) const {
    triangles_[index].interpolate_data(uv, n, t, tc);
}

template <typename Triangle>
float2 Data_interleaved<Triangle>::interpolate_uv(uint32_t index, float2 uv) const {
    return triangles_[index].interpolate_uv(uv);
}

template <typename Triangle>
float Data_interleaved<Triangle>::bitangent_sign(uint32_t index) const {
    return triangles_[index].bitangent_sign;
}

template <typename Triangle>
uint32_t Data_interleaved<Triangle>::material_index(uint32_t index) const {
    return triangles_[index].material_index;
}

template <typename Triangle>
float3 Data_interleaved<Triangle>::normal(uint32_t index) const {
    return triangles_[index].normal();
}

template <typename Triangle>
float Data_interleaved<Triangle>::area(uint32_t index) const {
    return triangles_[index].area();
}

template <typename Triangle>
float Data_interleaved<Triangle>::area(uint32_t index, float3 const& scale) const {
    return triangles_[index].area(scale);
}

/*
template<typename Triangle>
void Data_interleaved<Triangle>::sample(uint32_t index, float2 r2,
                                                                        float3& p, float3& n,
float2& tc) const { triangles_[index].interpolate(math::sample_triangle_uniform(r2), p, n, tc);
}*/

template <typename Triangle>
void Data_interleaved<Triangle>::sample(uint32_t index, float2 r2, float3& p, float2& tc) const {
    SOFT_ASSERT(index < triangles_.size());

    triangles_[index].interpolate(sample_triangle_uniform(r2), p, tc);
}

/*
template<typename Triangle>
void Data_interleaved<Triangle>::sample(uint32_t index, float2 r2, float3& p) const {
        triangles_[index].interpolate(math::sample_triangle_uniform(r2), p);
}
*/
template <typename Triangle>
void Data_interleaved<Triangle>::allocate_triangles(uint32_t num_triangles,
                                                    const std::vector<Vertex>& /*vertices*/) {
    triangles_.clear();
    triangles_.reserve(num_triangles);
}

template <typename Triangle>
void Data_interleaved<Triangle>::add_triangle(uint32_t a, uint32_t b, uint32_t c,
                                              uint32_t                   material_index,
                                              const std::vector<Vertex>& vertices) {
    float bitanget_sign = 1.f;

    if ((vertices[a].bitangent_sign < 0.f && vertices[b].bitangent_sign < 0.f) ||
        (vertices[b].bitangent_sign < 0.f && vertices[c].bitangent_sign < 0.f) ||
        (vertices[c].bitangent_sign < 0.f && vertices[a].bitangent_sign < 0.f)) {
        bitanget_sign = -1.f;
    }

    triangles_.push_back(
        Triangle(vertices[a], vertices[b], vertices[c], bitanget_sign, material_index));
}

template <typename Triangle>
size_t Data_interleaved<Triangle>::num_bytes() const {
    return triangles_.size() * sizeof(Triangle);
}

}  // namespace scene::shape::triangle::bvh

#endif
