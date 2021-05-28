#ifndef SU_CORE_SCENE_SHAPE_TRIANGLE_BVH_INDEXED_DATA_INL
#define SU_CORE_SCENE_SHAPE_TRIANGLE_BVH_INDEXED_DATA_INL

#include "base/math/quaternion.inl"
#include "base/math/sampling.inl"
#include "scene/material/material.hpp"
#include "scene/scene_worker.inl"
#include "scene/shape/shape_vertex.hpp"
#include "scene/shape/triangle/triangle_primitive_mt.inl"
#include "triangle_bvh_indexed_data.hpp"

#include "base/debug/assert.hpp"

#include <iostream>

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

inline bool Indexed_data::intersect(SimdVec origin, SimdVec direction, Simdf min_t, Simdf& max_t,
                                    uint32_t begin, uint32_t end, Simdf& u, Simdf& v,
                                    uint32_t& index) const {
    bool hit = false;

    alignas(16) float as[12];
    alignas(16) float bs[12];
    alignas(16) float cs[12];

    for (uint32_t j = begin; j < end;) {
        uint32_t const n = std::min(end - j, 4u);

        uint32_t const quad = j;

        for (uint32_t i = 0; i < n; ++i, ++j) {
            auto const tri = triangles_[j];

            float const* a = positions_[tri.a].v;
            float const* b = positions_[tri.b].v;
            float const* c = positions_[tri.c].v;

            as[0 + i] = a[0];
            as[4 + i] = a[1];
            as[8 + i] = a[2];

            bs[0 + i] = b[0];
            bs[4 + i] = b[1];
            bs[8 + i] = b[2];

            cs[0 + i] = c[0];
            cs[4 + i] = c[1];
            cs[8 + i] = c[2];
        }

        SimdVec a = {Simdf(&as[0]), Simdf(&as[4]), Simdf(&as[8])};
        SimdVec b = {Simdf(&bs[0]), Simdf(&bs[4]), Simdf(&bs[8])};
        SimdVec c = {Simdf(&cs[0]), Simdf(&cs[4]), Simdf(&cs[8])};

        uint32_t local_index;
        if (triangle::intersect(origin, direction, min_t, max_t, a, b, c, u, v, n - 1,
                                local_index)) {
            index = quad + local_index;
            hit   = true;
        }
    }

    return hit;
}

static inline bool visibility_helper(float3_p ray_dir, float2 uv, uint32_t index, uint32_t entity, Indexed_data::Filter filter , Indexed_data const& data, Worker& worker,
                                     float3& vis) {
    float3 const n = float3(data.normal(index));

     auto const material = worker.scene().prop_material(entity, data.part(index));

     return material->visibility(ray_dir, n, uv, filter, worker, vis);
}

inline bool Indexed_data::visibility(SimdVec origin, SimdVec direction, Simdf min_t, Simdf max_t,
                                     uint32_t begin, uint32_t end, float3_p ray_dir,
                                     uint32_t entity, Filter filter, Worker& worker,
                                     float3& vis) const {
    Simdf u;
    Simdf v;

    float3 local_vis(1.f);

    alignas(16) float as[12];
    alignas(16) float bs[12];
    alignas(16) float cs[12];

    for (uint32_t j = begin; j < end;) {
        uint32_t const n = std::min(end - j, 4u);

        uint32_t const quad = j;

        for (uint32_t i = 0; i < n; ++i, ++j) {
            auto const tri = triangles_[j];

            float const* a = positions_[tri.a].v;
            float const* b = positions_[tri.b].v;
            float const* c = positions_[tri.c].v;

            as[0 + i] = a[0];
            as[4 + i] = a[1];
            as[8 + i] = a[2];

            bs[0 + i] = b[0];
            bs[4 + i] = b[1];
            bs[8 + i] = b[2];

            cs[0 + i] = c[0];
            cs[4 + i] = c[1];
            cs[8 + i] = c[2];
        }

        SimdVec a = {Simdf(&as[0]), Simdf(&as[4]), Simdf(&as[8])};
        SimdVec b = {Simdf(&bs[0]), Simdf(&bs[4]), Simdf(&bs[8])};
        SimdVec c = {Simdf(&cs[0]), Simdf(&cs[4]), Simdf(&cs[8])};

        if (uint32_t hit = triangle::intersect2(origin, direction, min_t, max_t, a, b, c, u, v, n - 1); 0 != hit) {
            if (0 != (triangle::Select[0] & hit)) {
                uint32_t index = quad + 0;

                float2 const uv = interpolate_uv(u.splat_x(), v.splat_x(), index);

                float3 tv;
                if (!visibility_helper(ray_dir, uv, index, entity, filter, *this, worker, tv)) {
                    return false;
                }

                local_vis *= tv;
            }

            if ((0 != (triangle::Select[1] & hit)) & (n > 1)) {
                uint32_t index = quad + 1;

                float2 const uv = interpolate_uv(u.splat_y(), v.splat_y(), index);

                float3 tv;
                if (!visibility_helper(ray_dir, uv, index, entity, filter, *this, worker, tv)) {
                    return false;
                }

                local_vis *= tv;
            }

            if ((0 != (triangle::Select[2] & hit)) & (n > 2)) {
                uint32_t index = quad + 2;

                float2 const uv = interpolate_uv(u.splat_z(), v.splat_z(), index);

                float3 tv;
                if (!visibility_helper(ray_dir, uv, index, entity, filter, *this, worker, tv)) {
                    return false;
                }

                local_vis *= tv;
            }

            if ((0 != (triangle::Select[3] & hit)) & (n > 3)) {
                uint32_t index = quad + 3;

                float2 const uv = interpolate_uv(u.splat_w(), v.splat_w(), index);

                float3 tv;
                if (!visibility_helper(ray_dir, uv, index, entity, filter, *this, worker, tv)) {
                    return false;
                }

                local_vis *= tv;
            }
        }
    }

    vis = local_vis;
    return true;
}

inline bool Indexed_data::intersect_p(SimdVec origin, SimdVec direction, Simdf min_t, Simdf max_t,
                                      uint32_t begin, uint32_t end) const {
    alignas(16) float as[12];
    alignas(16) float bs[12];
    alignas(16) float cs[12];

    for (uint32_t j = begin; j < end;) {
        uint32_t const n = std::min(end - j, 4u);

        for (uint32_t i = 0; i < n; ++i, ++j) {
            auto const tri = triangles_[j];

            float const* a = positions_[tri.a].v;
            float const* b = positions_[tri.b].v;
            float const* c = positions_[tri.c].v;

            as[0 + i] = a[0];
            as[4 + i] = a[1];
            as[8 + i] = a[2];

            bs[0 + i] = b[0];
            bs[4 + i] = b[1];
            bs[8 + i] = b[2];

            cs[0 + i] = c[0];
            cs[4 + i] = c[1];
            cs[8 + i] = c[2];
        }

        SimdVec a = {Simdf(&as[0]), Simdf(&as[4]), Simdf(&as[8])};
        SimdVec b = {Simdf(&bs[0]), Simdf(&bs[4]), Simdf(&bs[8])};
        SimdVec c = {Simdf(&cs[0]), Simdf(&cs[4]), Simdf(&cs[8])};

        if (triangle::intersect_p(origin, direction, min_t, max_t, a, b, c, n - 1)) {
            return true;
        }
    }

    return false;
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

    auto const tri = triangles_[index];

    float3 const ia = positions_[tri.a];
    float3 const ib = positions_[tri.b];
    float3 const ic = positions_[tri.c];

    triangle::interpolate_p(ia, ib, ic, uv, p);

    SV const& sa = shading_vertices_[tri.a];
    SV const& sb = shading_vertices_[tri.b];
    SV const& sc = shading_vertices_[tri.c];

    tc = triangle::interpolate_uv(sa, sb, sc, uv);
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
