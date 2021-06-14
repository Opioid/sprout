#ifndef SU_CORE_SCENE_SHAPE_TRIANGLE_BVH_INDEXED_DATA_INL
#define SU_CORE_SCENE_SHAPE_TRIANGLE_BVH_INDEXED_DATA_INL

#include "base/math/quaternion.inl"
#include "base/math/sampling.inl"
#include "scene/shape/shape_vertex.hpp"
#include "scene/shape/triangle/triangle_primitive_mt.inl"
#include "triangle_bvh_indexed_data.hpp"

#include "base/debug/assert.hpp"

namespace scene::shape::triangle::bvh {

inline Indexed_data::Indexed_data()
    : num_triangles_(0),
      num_vertices_(0),
      triangles_(nullptr),
      positions_(nullptr),
      frames_(nullptr),
      uvs_(nullptr) {}

inline Indexed_data::~Indexed_data() {
    delete[] uvs_;
    delete[] frames_;
    delete[] positions_;
    delete[] triangles_;
}

inline uint32_t Indexed_data::num_triangles() const {
    return num_triangles_;
}

inline bool Indexed_data::intersect(Simdf_p origin, Simdf_p direction, scalar_p min_t,
                                    scalar& max_t, uint32_t index, scalar& u, scalar& v) const {
    auto const tri = triangles_[index];

    Simdf const a(positions_[tri.a].v);
    Simdf const b(positions_[tri.b].v);
    Simdf const c(positions_[tri.c].v);

    return triangle::intersect(origin, direction, min_t, max_t, a, b, c, u, v);
}

inline bool Indexed_data::intersect(Simdf_p origin, Simdf_p direction, scalar_p min_t,
                                    scalar& max_t, uint32_t index, uint32_t frame, Simdf_p weight,
                                    scalar& u, scalar& v) const {
    auto const tri = triangles_[index];

    uint32_t const o0 = (frame)*num_vertices_;

    Simdf const a0(positions_[tri.a + o0].v);
    Simdf const b0(positions_[tri.b + o0].v);
    Simdf const c0(positions_[tri.c + o0].v);

    uint32_t const o1 = (frame + 1) * num_vertices_;

    Simdf const a1(positions_[tri.a + o1].v);
    Simdf const b1(positions_[tri.b + o1].v);
    Simdf const c1(positions_[tri.c + o1].v);

    Simdf const a = lerp(a0, a1, weight);
    Simdf const b = lerp(b0, b1, weight);
    Simdf const c = lerp(c0, c1, weight);

    return triangle::intersect(origin, direction, min_t, max_t, a, b, c, u, v);
}

inline bool Indexed_data::intersect_p(Simdf_p origin, Simdf_p direction, scalar_p min_t,
                                      scalar_p max_t, uint32_t index) const {
    auto const tri = triangles_[index];

    Simdf const a(positions_[tri.a].v);
    Simdf const b(positions_[tri.b].v);
    Simdf const c(positions_[tri.c].v);

    return triangle::intersect_p(origin, direction, min_t, max_t, a, b, c);
}

inline bool Indexed_data::intersect_p(Simdf_p origin, Simdf_p direction, scalar_p min_t,
                                      scalar_p max_t, uint32_t index, uint32_t frame,
                                      Simdf_p weight) const {
    auto const tri = triangles_[index];

    uint32_t const o0 = (frame)*num_vertices_;

    Simdf const a0(positions_[tri.a + o0].v);
    Simdf const b0(positions_[tri.b + o0].v);
    Simdf const c0(positions_[tri.c + o0].v);

    uint32_t const o1 = (frame + 1) * num_vertices_;

    Simdf const a1(positions_[tri.a + o1].v);
    Simdf const b1(positions_[tri.b + o1].v);
    Simdf const c1(positions_[tri.c + o1].v);

    Simdf const a = lerp(a0, a1, weight);
    Simdf const b = lerp(b0, b1, weight);
    Simdf const c = lerp(c0, c1, weight);

    return triangle::intersect_p(origin, direction, min_t, max_t, a, b, c);
}

inline Simdf Indexed_data::interpolate_p(Simdf_p u, Simdf_p v, uint32_t index) const {
    auto const tri = triangles_[index];

    Simdf const ap(positions_[tri.a].v);
    Simdf const bp(positions_[tri.b].v);
    Simdf const cp(positions_[tri.c].v);

    return triangle::interpolate_p(ap, bp, cp, u, v);
}

inline Simdf Indexed_data::interpolate_p(Simdf_p u, Simdf_p v, uint32_t index, uint32_t frame,
                                         Simdf_p weight) const {
    auto const tri = triangles_[index];

    uint32_t const o0 = (frame)*num_vertices_;

    Simdf const a0(positions_[tri.a + o0].v);
    Simdf const b0(positions_[tri.b + o0].v);
    Simdf const c0(positions_[tri.c + o0].v);

    uint32_t const o1 = (frame + 1) * num_vertices_;

    Simdf const a1(positions_[tri.a + o1].v);
    Simdf const b1(positions_[tri.b + o1].v);
    Simdf const c1(positions_[tri.c + o1].v);

    Simdf const a = lerp(a0, a1, weight);
    Simdf const b = lerp(b0, b1, weight);
    Simdf const c = lerp(c0, c1, weight);

    return triangle::interpolate_p(a, b, c, u, v);
}

inline void Indexed_data::interpolate_data(Simdf_p u, Simdf_p v, uint32_t index, Simdf& n, Simdf& t,
                                           float2& tc) const {
    auto const tri = triangles_[index];

    auto const tna = quaternion::create_tangent_normal(frames_[tri.a]);
    auto const tnb = quaternion::create_tangent_normal(frames_[tri.b]);
    auto const tnc = quaternion::create_tangent_normal(frames_[tri.c]);

    float2 const uva = uvs_[tri.a];
    float2 const uvb = uvs_[tri.b];
    float2 const uvc = uvs_[tri.c];

    Shading_vertex_MTC const sa(tna.b, tna.a, uva);
    Shading_vertex_MTC const sb(tnb.b, tnb.a, uvb);
    Shading_vertex_MTC const sc(tnc.b, tnc.a, uvc);

    triangle::interpolate_data(u, v, sa, sb, sc, n, t, tc);
}

inline void Indexed_data::interpolate_data(Simdf_p u, Simdf_p v, uint32_t index, uint32_t frame,
                                           Simdf_p weight, Simdf& n, Simdf& t, float2& tc) const {
    auto const tri = triangles_[index];

    uint32_t const o0 = (frame)*num_vertices_;

    auto const fa0 = frames_[tri.a + o0];
    auto const fb0 = frames_[tri.b + o0];
    auto const fc0 = frames_[tri.c + o0];

    uint32_t const o1 = (frame + 1) * num_vertices_;

    auto const fa1 = frames_[tri.a + o1];
    auto const fb1 = frames_[tri.b + o1];
    auto const fc1 = frames_[tri.c + o1];

    float const w = weight.x();

    auto const fa = quaternion::slerp(fa0, fa1, w);
    auto const fb = quaternion::slerp(fb0, fb1, w);
    auto const fc = quaternion::slerp(fc0, fc1, w);

    auto const tna = quaternion::create_tangent_normal(fa);
    auto const tnb = quaternion::create_tangent_normal(fb);
    auto const tnc = quaternion::create_tangent_normal(fc);

    float2 const uva0 = uvs_[tri.a + o0];
    float2 const uvb0 = uvs_[tri.b + o0];
    float2 const uvc0 = uvs_[tri.c + o0];

    float2 const uva1 = uvs_[tri.a + o1];
    float2 const uvb1 = uvs_[tri.b + o1];
    float2 const uvc1 = uvs_[tri.c + o1];

    float2 const uva = lerp(uva0, uva1, w);
    float2 const uvb = lerp(uvb0, uvb1, w);
    float2 const uvc = lerp(uvc0, uvc1, w);

    Shading_vertex_MTC const sa(tna.b, tna.a, uva);
    Shading_vertex_MTC const sb(tnb.b, tnb.a, uvb);
    Shading_vertex_MTC const sc(tnc.b, tnc.a, uvc);

    triangle::interpolate_data(u, v, sa, sb, sc, n, t, tc);
}

inline Simdf Indexed_data::interpolate_shading_normal(Simdf_p u, Simdf_p v, uint32_t index) const {
    auto const tri = triangles_[index];

    float3 const a = quaternion::create_normal(frames_[tri.a]);
    float3 const b = quaternion::create_normal(frames_[tri.b]);
    float3 const c = quaternion::create_normal(frames_[tri.c]);

    return triangle::interpolate_normal(u, v, a, b, c);
}

inline Simdf Indexed_data::interpolate_shading_normal(Simdf_p u, Simdf_p v, uint32_t index,
                                                      uint32_t frame, Simdf_p weight) const {
    auto const tri = triangles_[index];

    uint32_t const o0 = (frame)*num_vertices_;

    auto const fa0 = frames_[tri.a + o0];
    auto const fb0 = frames_[tri.b + o0];
    auto const fc0 = frames_[tri.c + o0];

    uint32_t const o1 = (frame + 1) * num_vertices_;

    auto const fa1 = frames_[tri.a + o1];
    auto const fb1 = frames_[tri.b + o1];
    auto const fc1 = frames_[tri.c + o1];

    float const w = weight.x();

    auto const fa = quaternion::slerp(fa0, fa1, w);
    auto const fb = quaternion::slerp(fb0, fb1, w);
    auto const fc = quaternion::slerp(fc0, fc1, w);

    float3 const a = quaternion::create_normal(fa);
    float3 const b = quaternion::create_normal(fb);
    float3 const c = quaternion::create_normal(fc);

    return triangle::interpolate_normal(u, v, a, b, c);
}

inline float2 Indexed_data::interpolate_uv(Simdf_p u, Simdf_p v, uint32_t index) const {
    auto const tri = triangles_[index];

    float2 const a = uvs_[tri.a];
    float2 const b = uvs_[tri.b];
    float2 const c = uvs_[tri.c];

    return triangle::interpolate_uv(u, v, a, b, c);
}

inline float2 Indexed_data::interpolate_uv(Simdf_p u, Simdf_p v, uint32_t index, uint32_t frame,
                                           Simdf_p weight) const {
    auto const tri = triangles_[index];

    uint32_t const o0 = (frame)*num_vertices_;

    float2 const a0 = uvs_[tri.a + o0];
    float2 const b0 = uvs_[tri.b + o0];
    float2 const c0 = uvs_[tri.c + o0];

    uint32_t const o1 = (frame + 1) * num_vertices_;

    float2 const a1 = uvs_[tri.a + o1];
    float2 const b1 = uvs_[tri.b + o1];
    float2 const c1 = uvs_[tri.c + o1];

    float const w = weight.x();

    float2 const a = lerp(a0, a1, w);
    float2 const b = lerp(b0, b1, w);
    float2 const c = lerp(c0, c1, w);
    return triangle::interpolate_uv(u, v, a, b, c);
}

inline float Indexed_data::bitangent_sign(uint32_t index) const {
    return 0 == triangles_[index].bts ? 1.f : -1.f;
    // return frames_[index][3] < 0.f ? -1.f : 1.f;
}

inline uint32_t Indexed_data::part(uint32_t index) const {
    return triangles_[index].part;
}

inline Simdf Indexed_data::normal(uint32_t index) const {
    auto const tri = triangles_[index];

    Simdf const a(positions_[tri.a].v);
    Simdf const b(positions_[tri.b].v);
    Simdf const c(positions_[tri.c].v);

    Simdf const e1 = b - a;
    Simdf const e2 = c - a;

    return normalize3(cross3(e1, e2));
}

inline Simdf Indexed_data::normal(uint32_t index, uint32_t frame, Simdf_p weight) const {
    auto const tri = triangles_[index];

    uint32_t const o0 = (frame)*num_vertices_;

    Simdf const a0(positions_[tri.a + o0].v);
    Simdf const b0(positions_[tri.b + o0].v);
    Simdf const c0(positions_[tri.c + o0].v);

    uint32_t const o1 = (frame + 1) * num_vertices_;

    Simdf const a1(positions_[tri.a + o1].v);
    Simdf const b1(positions_[tri.b + o1].v);
    Simdf const c1(positions_[tri.c + o1].v);

    Simdf const a = lerp(a0, a1, weight);
    Simdf const b = lerp(b0, b1, weight);
    Simdf const c = lerp(c0, c1, weight);

    Simdf const e1 = b - a;
    Simdf const e2 = c - a;

    return normalize3(cross3(e1, e2));
}

inline float Indexed_data::area(uint32_t index) const {
    auto const tri = triangles_[index];

    float3 const a = positions_[tri.a];
    float3 const b = positions_[tri.b];
    float3 const c = positions_[tri.c];

    return triangle::area(a, b, c);
}

inline void Indexed_data::triangle(uint32_t index, float3& pa, float3& pb, float3& pc) const {
    auto const tri = triangles_[index];

    pa = positions_[tri.a];
    pb = positions_[tri.b];
    pc = positions_[tri.c];
}

inline void Indexed_data::triangle(uint32_t index, float3& pa, float3& pb, float3& pc, float2& uva,
                                   float2& uvb, float2& uvc) const {
    auto const tri = triangles_[index];

    pa = positions_[tri.a];
    pb = positions_[tri.b];
    pc = positions_[tri.c];

    uva = uvs_[tri.a];
    uvb = uvs_[tri.b];
    uvc = uvs_[tri.c];
}

inline void Indexed_data::sample(uint32_t index, float2 r2, float3& p, float2& tc) const {
    SOFT_ASSERT(index < num_triangles_);

    float2 const uv = sample_triangle_uniform(r2);

    Simdf const u(uv[0]);
    Simdf const v(uv[1]);

    auto const tri = triangles_[index];

    Simdf const ia = Simdf(positions_[tri.a]);
    Simdf const ib = Simdf(positions_[tri.b]);
    Simdf const ic = Simdf(positions_[tri.c]);

    p = float3(triangle::interpolate_p(ia, ib, ic, u, v));

    float2 const sa = uvs_[tri.a];
    float2 const sb = uvs_[tri.b];
    float2 const sc = uvs_[tri.c];

    tc = triangle::interpolate_uv(u, v, sa, sb, sc);
}

inline void Indexed_data::allocate_triangles(uint32_t num_triangles, uint32_t num_frames,
                                             Vertex_stream const& vertices) {
    uint32_t const num_vertices       = vertices.num_vertices();
    uint32_t const num_total_vertices = num_frames * num_vertices;

    if (num_triangles != num_triangles_ || num_vertices != num_vertices_) {
        num_triangles_ = num_triangles;
        num_vertices_  = num_vertices;

        delete[] uvs_;
        delete[] frames_;
        delete[] positions_;
        delete[] triangles_;

        triangles_ = new Index_triangle[num_triangles];
        positions_ = new float3[num_total_vertices];
        frames_    = new float4[num_total_vertices];
        uvs_       = new float2[num_total_vertices];
    }

    for (uint32_t i = 0; i < num_total_vertices; ++i) {
        positions_[i] = vertices.p(i);

        frames_[i] = vertices.frame(i);

        uvs_[i] = vertices.uv(i);
    }
}

inline void Indexed_data::set_triangle(uint32_t a, uint32_t b, uint32_t c, uint32_t part,
                                       Vertex_stream const& vertices, uint32_t triangle_id) {
    bool const abts = vertices.bitangent_sign(a);
    bool const bbts = vertices.bitangent_sign(b);
    bool const cbts = vertices.bitangent_sign(c);

    bool const bitanget_sign = (abts && bbts) || (bbts && cbts) || (cbts && abts);

    triangles_[triangle_id] = Index_triangle(a, b, c, bitanget_sign, part);
}

inline Indexed_data::Index_triangle::Index_triangle() = default;

inline Indexed_data::Index_triangle::Index_triangle(uint32_t a, uint32_t b, uint32_t c,
                                                    bool bitangent_sign, uint32_t part)
    : a(a), b(b), c(c), bts(bitangent_sign ? 1u : 0u), part(part) {}

}  // namespace scene::shape::triangle::bvh

#endif
