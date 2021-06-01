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
      shading_vertices_(nullptr) {}

inline Indexed_data::~Indexed_data() {
    delete[] shading_vertices_;
    delete[] positions_;
    delete[] triangles_;
}

inline uint32_t Indexed_data::num_triangles() const {
    return num_triangles_;
}

inline bool Indexed_data::intersect(Simdf_p origin, Simdf_p direction, scalar_p min_t,
                                    scalar& max_t, uint32_t index, scalar& u, scalar& v) const {
    auto const tri = triangles_[index];

    float const* a = positions_[tri.a].v;
    float const* b = positions_[tri.b].v;
    float const* c = positions_[tri.c].v;

    return triangle::intersect(origin, direction, min_t, max_t, a, b, c, u, v);
}

inline bool Indexed_data::intersect_p(Simdf_p origin, Simdf_p direction, scalar_p min_t,
                                      scalar_p max_t, uint32_t index) const {
    auto const tri = triangles_[index];

    float const* a = positions_[tri.a].v;
    float const* b = positions_[tri.b].v;
    float const* c = positions_[tri.c].v;

    return triangle::intersect_p(origin, direction, min_t, max_t, a, b, c);
}

inline Simdf Indexed_data::interpolate_p(Simdf_p u, Simdf_p v, uint32_t index) const {
    auto const tri = triangles_[index];

    Simdf const ap(positions_[tri.a].v);
    Simdf const bp(positions_[tri.b].v);
    Simdf const cp(positions_[tri.c].v);

    return triangle::interpolate_p(ap, bp, cp, u, v);
}

inline void Indexed_data::interpolate_data(Simdf_p u, Simdf_p v, uint32_t index, Simdf& n, Simdf& t,
                                           float2& tc) const {
    auto const tri = triangles_[index];

    SV const& a = shading_vertices_[tri.a];
    SV const& b = shading_vertices_[tri.b];
    SV const& c = shading_vertices_[tri.c];

    triangle::interpolate_data(u, v, a, b, c, n, t, tc);
}

inline Simdf Indexed_data::interpolate_shading_normal(Simdf_p u, Simdf_p v, uint32_t index) const {
    auto const tri = triangles_[index];

    SV const& a = shading_vertices_[tri.a];
    SV const& b = shading_vertices_[tri.b];
    SV const& c = shading_vertices_[tri.c];

    return triangle::interpolate_normal(u, v, a, b, c);
}

inline float2 Indexed_data::interpolate_uv(Simdf_p u, Simdf_p v, uint32_t index) const {
    auto const tri = triangles_[index];

    SV const& a = shading_vertices_[tri.a];
    SV const& b = shading_vertices_[tri.b];
    SV const& c = shading_vertices_[tri.c];

    return triangle::interpolate_uv(u, v, a, b, c);
}

inline float Indexed_data::bitangent_sign(uint32_t index) const {
    return 0 == triangles_[index].bts ? 1.f : -1.f;
}

inline uint32_t Indexed_data::part(uint32_t index) const {
    return triangles_[index].part;
}

inline Simdf Indexed_data::normal(uint32_t index) const {
    auto const tri = triangles_[index];

    Simdf const ap(positions_[tri.a].v);
    Simdf const bp(positions_[tri.b].v);
    Simdf const cp(positions_[tri.c].v);

    Simdf const e1 = bp - ap;
    Simdf const e2 = cp - ap;

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

    uva = float2(shading_vertices_[tri.a].n_u[3], shading_vertices_[tri.a].t_v[3]);
    uvb = float2(shading_vertices_[tri.b].n_u[3], shading_vertices_[tri.b].t_v[3]);
    uvc = float2(shading_vertices_[tri.c].n_u[3], shading_vertices_[tri.c].t_v[3]);
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

    SV const& sa = shading_vertices_[tri.a];
    SV const& sb = shading_vertices_[tri.b];
    SV const& sc = shading_vertices_[tri.c];

    tc = triangle::interpolate_uv(u, v, sa, sb, sc);
}

inline void Indexed_data::allocate_triangles(uint32_t             num_triangles,
                                             Vertex_stream const& vertices) {
    uint32_t const num_vertices = vertices.num_vertices();
    if (num_triangles != num_triangles_ || num_vertices != num_vertices_) {
        num_triangles_ = num_triangles;
        num_vertices_  = num_vertices;

        delete[] shading_vertices_;
        delete[] positions_;
        delete[] triangles_;

        triangles_        = new Index_triangle[num_triangles];
        positions_        = new float3[num_vertices];
        shading_vertices_ = new SV[num_vertices];
    }

    for (uint32_t i = 0; i < num_vertices; ++i) {
        positions_[i] = vertices.p(i);

        auto const [n, t] = vertices.nt(i);

        shading_vertices_[i] = SV(n, t, vertices.uv(i));
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
