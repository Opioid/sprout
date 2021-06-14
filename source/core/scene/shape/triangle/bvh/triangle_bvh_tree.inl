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

inline Simdf Tree::interpolate_p(Simdf_p u, Simdf_p v, uint32_t index) const {
    return data_.interpolate_p(u, v, index);
}

inline Simdf Tree::interpolate_p(Simdf_p u, Simdf_p v, uint32_t index, uint32_t frame,
                                 Simdf_p weight) const {
    return data_.interpolate_p(u, v, index, frame, weight);
}

inline void Tree::interpolate_triangle_data(Simdf_p u, Simdf_p v, uint32_t index, Simdf& n,
                                            Simdf& t, float2& tc) const {
    data_.interpolate_data(u, v, index, n, t, tc);
}

inline void Tree::interpolate_triangle_data(Simdf_p u, Simdf_p v, uint32_t index, uint32_t frame,
                                            Simdf_p weight, Simdf& n, Simdf& t, float2& tc) const {
    data_.interpolate_data(u, v, index, frame, weight, n, t, tc);
}

inline Simdf Tree::interpolate_shading_normal(Simdf_p u, Simdf_p v, uint32_t index) const {
    return data_.interpolate_shading_normal(u, v, index);
}

inline Simdf Tree::interpolate_shading_normal(Simdf_p u, Simdf_p v, uint32_t index, uint32_t frame,
                                              Simdf_p weight) const {
    return data_.interpolate_shading_normal(u, v, index, frame, weight);
}

inline float2 Tree::interpolate_triangle_uv(Simdf_p u, Simdf_p v, uint32_t index) const {
    return data_.interpolate_uv(u, v, index);
}

inline float2 Tree::interpolate_triangle_uv(Simdf_p u, Simdf_p v, uint32_t index, uint32_t frame,
                                            Simdf_p weight) const {
    return data_.interpolate_uv(u, v, index, frame, weight);
}

inline float Tree::triangle_bitangent_sign(uint32_t index) const {
    return data_.bitangent_sign(index);
}

inline uint32_t Tree::triangle_part(uint32_t index) const {
    return data_.part(index);
}

inline Simdf Tree::triangle_normal(uint32_t index) const {
    return data_.normal(index);
}

inline Simdf Tree::triangle_normal(uint32_t index, uint32_t frame, Simdf_p weight) const {
    return data_.normal(index, frame, weight);
}

inline float Tree::triangle_area(uint32_t index) const {
    return data_.area(index);
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

inline void Tree::allocate_triangles(uint32_t num_triangles, uint32_t num_frames,
                                     Vertex_stream const& vertices) {
    data_.allocate_triangles(num_triangles, num_frames, vertices);
}

inline void Tree::set_triangle(uint32_t a, uint32_t b, uint32_t c, uint32_t part,
                               Vertex_stream const& vertices, uint32_t triangle_id) {
    data_.set_triangle(a, b, c, part, vertices, triangle_id);
}

}  // namespace scene::shape::triangle::bvh

#endif
