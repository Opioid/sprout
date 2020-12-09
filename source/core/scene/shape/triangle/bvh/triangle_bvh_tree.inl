#ifndef SU_CORE_SCENE_SHAPE_TRIANGLE_BVH_TREE_INL
#define SU_CORE_SCENE_SHAPE_TRIANGLE_BVH_TREE_INL

#include "triangle_bvh_indexed_data.inl"
#include "triangle_bvh_tree.hpp"

namespace scene::shape::triangle::bvh {

inline uint32_t Tree::num_parts() const {
    return num_parts_;
}

inline uint32_t Tree::num_triangles() const {
    return data_.num_triangles();
}

inline float3 Tree::interpolate_p(float2 uv, uint32_t index) const {
    return data_.interpolate_p(uv, index);
}

inline Simd3f Tree::interpolate_p(Simd3f_p u, Simd3f_p v, uint32_t index) const {
    return data_.interpolate_p(u, v, index);
}

inline void Tree::interpolate_triangle_data(Simd3f_p u, Simd3f_p v, uint32_t index, Simd3f& n,
                                            Simd3f& t, float2& tc) const {
    data_.interpolate_data(u, v, index, n, t, tc);
}

inline Simd3f Tree::interpolate_shading_normal(Simd3f_p u, Simd3f_p v, uint32_t index) const {
    return data_.interpolate_shading_normal(u, v, index);
}

inline float2 Tree::interpolate_triangle_uv(Simd3f_p u, Simd3f_p v, uint32_t index) const {
    return data_.interpolate_uv(u, v, index);
}

inline float Tree::triangle_bitangent_sign(uint32_t index) const {
    return data_.bitangent_sign(index);
}

inline uint32_t Tree::triangle_part(uint32_t index) const {
    return data_.part(index);
}

inline float3 Tree::triangle_normal(uint32_t index) const {
    return data_.normal(index);
}

inline Simd3f Tree::triangle_normal_v(uint32_t index) const {
    return data_.normal_v(index);
}

inline float Tree::triangle_area(uint32_t index) const {
    return data_.area(index);
}

inline float Tree::triangle_area(uint32_t index, float3_p scale) const {
    return data_.area(index, scale);
}

inline float3 Tree::triangle_center(uint32_t index) const {
    return data_.center(index);
}

inline void Tree::triangle(uint32_t index, float3& pa, float3& pb, float3& pc) const {
    return data_.triangle(index, pa, pb, pc);
}

inline void Tree::triangle(uint32_t index, float3& pa, float3& pb, float3& pc, float2& uva,
                           float2& uvb, float2& uvc) const {
    return data_.triangle(index, pa, pb, pc, uva, uvb, uvc);
}

inline void Tree::sample(uint32_t index, float2 r2, float3& p, float2& tc) const {
    data_.sample(index, r2, p, tc);
}

inline void Tree::allocate_parts(uint32_t num_parts) {
    num_parts_ = num_parts;
}

inline void Tree::allocate_triangles(uint32_t num_triangles, Vertex_stream const& vertices) {
    data_.allocate_triangles(num_triangles, vertices);
}

inline void Tree::set_triangle(uint32_t a, uint32_t b, uint32_t c, uint32_t part,
                               Vertex_stream const& vertices, uint32_t triangle_id) {
    data_.set_triangle(a, b, c, part, vertices, triangle_id);
}

}  // namespace scene::shape::triangle::bvh

#endif
