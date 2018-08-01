#ifndef SU_CORE_SCENE_SHAPE_TRIANGLE_BVH_INDEXED_DATA_INL
#define SU_CORE_SCENE_SHAPE_TRIANGLE_BVH_INDEXED_DATA_INL

#include "base/math/sampling/sampling.hpp"
#include "base/memory/align.hpp"
#include "scene/shape/triangle/triangle_primitive_mt.inl"
#include "triangle_bvh_indexed_data.hpp"

#include "base/debug/assert.hpp"

namespace scene::shape::triangle::bvh {

template <typename SV>
Indexed_data<SV>::Indexed_data() noexcept
    : num_triangles_(0),
      current_triangle_(0),
      num_vertices_(0),
      triangles_(nullptr),
      intersection_vertices_(nullptr),
      shading_vertices_(nullptr) {}

template <typename SV>
Indexed_data<SV>::~Indexed_data() noexcept {
    memory::free_aligned(shading_vertices_);
    memory::free_aligned(intersection_vertices_);
    memory::free_aligned(triangles_);
}

template <typename SV>
uint32_t Indexed_data<SV>::num_triangles() const noexcept {
    return num_triangles_;
}

template <typename SV>
uint32_t Indexed_data<SV>::current_triangle() const noexcept {
    return current_triangle_;
}

template <typename SV>
bool Indexed_data<SV>::intersect(uint32_t index, math::Ray& ray, float2& uv) const noexcept {
    auto const t = triangles_[index];

    float3 const a = intersection_vertices_[t.a];
    float3 const b = intersection_vertices_[t.b];
    float3 const c = intersection_vertices_[t.c];

    return triangle::intersect(a, b, c, ray, uv);
}

template <typename SV>
bool Indexed_data<SV>::intersect_p(uint32_t index, math::Ray const& ray) const noexcept {
    auto const tri = triangles_[index];

    float3 const a = intersection_vertices_[tri.a];
    float3 const b = intersection_vertices_[tri.b];
    float3 const c = intersection_vertices_[tri.c];

    return triangle::intersect_p(a, b, c, ray);
}

template <typename SV>
bool Indexed_data<SV>::intersect(FVector origin, FVector direction, FVector min_t, Vector& max_t,
                                 uint32_t index, Vector& u, Vector& v) const noexcept {
    auto const tri = triangles_[index];

    float const* a = intersection_vertices_[tri.a].v;
    float const* b = intersection_vertices_[tri.b].v;
    float const* c = intersection_vertices_[tri.c].v;

    return triangle::intersect(origin, direction, min_t, max_t, a, b, c, u, v);
}

template <typename SV>
bool Indexed_data<SV>::intersect(FVector origin, FVector direction, FVector min_t, Vector& max_t,
                                 uint32_t index) const noexcept {
    auto const tri = triangles_[index];

    float const* a = intersection_vertices_[tri.a].v;
    float const* b = intersection_vertices_[tri.b].v;
    float const* c = intersection_vertices_[tri.c].v;

    return triangle::intersect(origin, direction, min_t, max_t, a, b, c);
}

template <typename SV>
bool Indexed_data<SV>::intersect_p(FVector origin, FVector direction, FVector min_t, FVector max_t,
                                   uint32_t index) const noexcept {
    auto const tri = triangles_[index];

    float const* a = intersection_vertices_[tri.a].v;
    float const* b = intersection_vertices_[tri.b].v;
    float const* c = intersection_vertices_[tri.c].v;

    return triangle::intersect_p(origin, direction, min_t, max_t, a, b, c);
}

template <typename SV>
void Indexed_data<SV>::interpolate_data(uint32_t index, float2 uv, float3& n, float3& t,
                                        float2& tc) const noexcept {
    auto const tri = triangles_[index];

    SV const& a = shading_vertices_[tri.a];
    SV const& b = shading_vertices_[tri.b];
    SV const& c = shading_vertices_[tri.c];

    triangle::interpolate_data(a, b, c, uv, n, t, tc);
}

template <typename SV>
void Indexed_data<SV>::interpolate_data(FVector u, FVector v, uint32_t index, float3& n, float3& t,
                                        float2& tc) const noexcept {
    auto const tri = triangles_[index];

    SV const& a = shading_vertices_[tri.a];
    SV const& b = shading_vertices_[tri.b];
    SV const& c = shading_vertices_[tri.c];

    triangle::interpolate_data(u, v, a, b, c, n, t, tc);
}

template <typename SV>
void Indexed_data<SV>::interpolate_data(FVector u, FVector v, uint32_t index, Vector& n, Vector& t,
                                        float2& tc) const noexcept {
    auto const tri = triangles_[index];

    SV const& a = shading_vertices_[tri.a];
    SV const& b = shading_vertices_[tri.b];
    SV const& c = shading_vertices_[tri.c];

    triangle::interpolate_data(u, v, a, b, c, n, t, tc);
}

template <typename SV>
float2 Indexed_data<SV>::interpolate_uv(uint32_t index, float2 uv) const noexcept {
    auto const tri = triangles_[index];

    SV const& sa = shading_vertices_[tri.a];
    SV const& sb = shading_vertices_[tri.b];
    SV const& sc = shading_vertices_[tri.c];

    return triangle::interpolate_uv(sa, sb, sc, uv);
}

template <typename SV>
float2 Indexed_data<SV>::interpolate_uv(FVector u, FVector v, uint32_t index) const noexcept {
    auto const tri = triangles_[index];

    SV const& sa = shading_vertices_[tri.a];
    SV const& sb = shading_vertices_[tri.b];
    SV const& sc = shading_vertices_[tri.c];

    return triangle::interpolate_uv(u, v, sa, sb, sc);
}

template <typename SV>
float Indexed_data<SV>::bitangent_sign(uint32_t index) const noexcept {
    static float constexpr signs[2] = {1.f, -1.f};

    return signs[triangles_[index].bts];
}

template <typename SV>
uint32_t Indexed_data<SV>::material_index(uint32_t index) const noexcept {
    return triangles_[index].material_index;
}

template <typename SV>
float3 Indexed_data<SV>::normal(uint32_t index) const noexcept {
    auto const tri = triangles_[index];

    float3 const a = intersection_vertices_[tri.a];
    float3 const b = intersection_vertices_[tri.b];
    float3 const c = intersection_vertices_[tri.c];

    float3 const e1 = b - a;
    float3 const e2 = c - a;
    return math::normalize(math::cross(e1, e2));
}

template <typename SV>
Vector Indexed_data<SV>::normal_v(uint32_t index) const noexcept {
    auto const tri = triangles_[index];

    Vector const ap = simd::load_float4(intersection_vertices_[tri.a].v);
    Vector const bp = simd::load_float4(intersection_vertices_[tri.b].v);
    Vector const cp = simd::load_float4(intersection_vertices_[tri.c].v);

    Vector const e1 = math::sub(bp, ap);
    Vector const e2 = math::sub(cp, ap);

    return math::normalized3(math::cross3(e1, e2));
}

template <typename SV>
float Indexed_data<SV>::area(uint32_t index) const noexcept {
    auto const tri = triangles_[index];

    float3 const a = intersection_vertices_[tri.a];
    float3 const b = intersection_vertices_[tri.b];
    float3 const c = intersection_vertices_[tri.c];

    return triangle::area(a, b, c);
}

template <typename SV>
float Indexed_data<SV>::area(uint32_t index, f_float3 scale) const noexcept {
    auto const tri = triangles_[index];

    float3 const a = intersection_vertices_[tri.a];
    float3 const b = intersection_vertices_[tri.b];
    float3 const c = intersection_vertices_[tri.c];

    return triangle::area(a, b, c, scale);
}

template <typename SV>
void Indexed_data<SV>::sample(uint32_t index, float2 r2, float3& p, float2& tc) const noexcept {
    SOFT_ASSERT(index < num_triangles_);

    float2 const uv = math::sample_triangle_uniform(r2);

    auto const tri = triangles_[index];

    float3 const ia = intersection_vertices_[tri.a];
    float3 const ib = intersection_vertices_[tri.b];
    float3 const ic = intersection_vertices_[tri.c];

    triangle::interpolate_p(ia, ib, ic, uv, p);

    SV const& sa = shading_vertices_[tri.a];
    SV const& sb = shading_vertices_[tri.b];
    SV const& sc = shading_vertices_[tri.c];

    tc = triangle::interpolate_uv(sa, sb, sc, uv);
}

template <typename SV>
void Indexed_data<SV>::allocate_triangles(uint32_t        num_triangles,
                                          Vertices const& vertices) noexcept {
    uint32_t const num_vertices = static_cast<uint32_t>(vertices.size());

    if (num_triangles != num_triangles_ || num_vertices != num_vertices_) {
        num_triangles_ = num_triangles;
        num_vertices_  = num_vertices;

        memory::free_aligned(shading_vertices_);
        memory::free_aligned(intersection_vertices_);
        memory::free_aligned(triangles_);

        triangles_             = memory::allocate_aligned<Index_triangle>(num_triangles);
        intersection_vertices_ = memory::allocate_aligned<float3>(num_vertices);
        shading_vertices_      = memory::allocate_aligned<SV>(num_vertices);
    }

    for (uint32_t i = 0; i < num_vertices; ++i) {
        intersection_vertices_[i] = float3(vertices[i].p);

        shading_vertices_[i] = SV(vertices[i].n, vertices[i].t, vertices[i].uv);
    }

    current_triangle_ = 0;
}

template <typename SV>
void Indexed_data<SV>::add_triangle(uint32_t a, uint32_t b, uint32_t c, uint32_t material_index,
                                    Vertices const& vertices) noexcept {
    float bitanget_sign = 1.f;

    if ((vertices[a].bitangent_sign < 0.f && vertices[b].bitangent_sign < 0.f) ||
        (vertices[b].bitangent_sign < 0.f && vertices[c].bitangent_sign < 0.f) ||
        (vertices[c].bitangent_sign < 0.f && vertices[a].bitangent_sign < 0.f)) {
        bitanget_sign = -1.f;
    }

    triangles_[current_triangle_] = Index_triangle(a, b, c, bitanget_sign, material_index);
    ++current_triangle_;
}

template <typename SV>
size_t Indexed_data<SV>::num_bytes() const noexcept {
    return sizeof(*this) + num_triangles_ * sizeof(Index_triangle) +
           num_vertices_ * (sizeof(float3) + sizeof(SV));
}

template <typename SV>
Indexed_data<SV>::Index_triangle::Index_triangle(uint32_t a, uint32_t b, uint32_t c,
                                                 float    bitangent_sign,
                                                 uint32_t material_index) noexcept
    : a(a), b(b), c(c), bts(bitangent_sign < 0.f ? 1 : 0), material_index(material_index) {}

template <typename V>
Indexed_data_interleaved<V>::Indexed_data_interleaved()
    : num_triangles_(0),
      current_triangle_(0),
      num_vertices_(0),
      triangles_(nullptr),
      vertices_(nullptr) {}

template <typename V>
Indexed_data_interleaved<V>::~Indexed_data_interleaved() {
    memory::free_aligned(vertices_);
    memory::free_aligned(triangles_);
}

template <typename V>
uint32_t Indexed_data_interleaved<V>::num_triangles() const {
    return num_triangles_;
}

template <typename V>
uint32_t Indexed_data_interleaved<V>::current_triangle() const {
    return current_triangle_;
}

template <typename V>
bool Indexed_data_interleaved<V>::intersect(uint32_t index, math::Ray& ray, float2& uv) const {
    auto const& t = triangles_[index];

    V const& a = vertices_[t.a];
    V const& b = vertices_[t.b];
    V const& c = vertices_[t.c];

    return triangle::intersect(a, b, c, ray, uv);
}

template <typename V>
bool Indexed_data_interleaved<V>::intersect_p(uint32_t index, math::Ray const& ray) const {
    auto const& tri = triangles_[index];

    V const& a = vertices_[tri.a];
    V const& b = vertices_[tri.b];
    V const& c = vertices_[tri.c];

    return triangle::intersect_p(a, b, c, ray);
}

template <typename V>
void Indexed_data_interleaved<V>::interpolate_data(uint32_t index, float2 uv, float3& n, float3& t,
                                                   float2& tc) const {
    auto const& tri = triangles_[index];

    V const& a = vertices_[tri.a];
    V const& b = vertices_[tri.b];
    V const& c = vertices_[tri.c];

    triangle::interpolate_data(a, b, c, uv, n, t, tc);
}

template <typename V>
float2 Indexed_data_interleaved<V>::interpolate_uv(uint32_t index, float2 uv) const {
    auto const& tri = triangles_[index];

    V const& sa = vertices_[tri.a];
    V const& sb = vertices_[tri.b];
    V const& sc = vertices_[tri.c];

    return triangle::interpolate_uv(sa, sb, sc, uv);
}

template <typename V>
float Indexed_data_interleaved<V>::bitangent_sign(uint32_t index) const {
    static float constexpr signs[2] = {1.f, -1.f};

    return signs[(Index_triangle::BTS_mask & triangles_[index].bts_material_index) >> 31];
}

template <typename V>
uint32_t Indexed_data_interleaved<V>::material_index(uint32_t index) const {
    return Index_triangle::Material_index_mask & triangles_[index].bts_material_index;
}

template <typename V>
float3 Indexed_data_interleaved<V>::normal(uint32_t index) const {
    auto const& tri = triangles_[index];

    V const& a = vertices_[tri.a];
    V const& b = vertices_[tri.b];
    V const& c = vertices_[tri.c];

    float3 e1 = b.p - a.p;
    float3 e2 = c.p - a.p;
    return math::normalize(math::cross(e1, e2));
}

template <typename V>
float Indexed_data_interleaved<V>::area(uint32_t index) const {
    auto const& tri = triangles_[index];

    V const& a = vertices_[tri.a];
    V const& b = vertices_[tri.b];
    V const& c = vertices_[tri.c];

    return triangle::area(a, b, c);
}

template <typename V>
float Indexed_data_interleaved<V>::area(uint32_t index, f_float3 scale) const {
    auto const& tri = triangles_[index];

    V const& a = vertices_[tri.a];
    V const& b = vertices_[tri.b];
    V const& c = vertices_[tri.c];

    return triangle::area(a, b, c, scale);
}

template <typename V>
void Indexed_data_interleaved<V>::sample(uint32_t index, float2 r2, float3& p, float2& tc) const {
    SOFT_ASSERT(index < num_triangles_);

    float2 const uv = math::sample_triangle_uniform(r2);

    auto const& tri = triangles_[index];

    V const& ia = vertices_[tri.a];
    V const& ib = vertices_[tri.b];
    V const& ic = vertices_[tri.c];

    triangle::interpolate_p_uv(ia, ib, ic, uv, p, tc);
}

template <typename V>
void Indexed_data_interleaved<V>::allocate_triangles(uint32_t        num_triangles,
                                                     Vertices const& vertices) {
    num_triangles_    = num_triangles;
    current_triangle_ = 0;
    num_vertices_     = static_cast<uint32_t>(vertices.size());

    memory::free_aligned(vertices_);
    memory::free_aligned(triangles_);

    triangles_ = memory::allocate_aligned<Index_triangle>(num_triangles);
    vertices_  = memory::allocate_aligned<V>(num_vertices_);

    for (uint32_t i = 0, len = num_vertices_; i < len; ++i) {
        vertices_[i] = V(vertices[i].p, vertices[i].n, vertices[i].t, vertices[i].uv);
    }
}

template <typename V>
void Indexed_data_interleaved<V>::add_triangle(uint32_t a, uint32_t b, uint32_t c,
                                               uint32_t material_index, Vertices const& vertices) {
    float bitanget_sign = 1.f;

    if ((vertices[a].bitangent_sign < 0.f && vertices[b].bitangent_sign < 0.f) ||
        (vertices[b].bitangent_sign < 0.f && vertices[c].bitangent_sign < 0.f) ||
        (vertices[c].bitangent_sign < 0.f && vertices[a].bitangent_sign < 0.f)) {
        bitanget_sign = -1.f;
    }

    triangles_[current_triangle_] = Index_triangle(a, b, c, bitanget_sign, material_index);
    ++current_triangle_;
}

template <typename V>
size_t Indexed_data_interleaved<V>::num_bytes() const {
    return sizeof(*this) + num_triangles_ * sizeof(Index_triangle) + num_vertices_ * sizeof(V);
}

template <typename V>
Indexed_data_interleaved<V>::Index_triangle::Index_triangle(uint32_t a, uint32_t b, uint32_t c,
                                                            float    bitangent_sign,
                                                            uint32_t material_index)
    : a(a),
      b(b),
      c(c),
      bts_material_index(bitangent_sign < 0.f ? BTS_mask | material_index : material_index) {}

template <typename IV, typename SV>
Hybrid_data<IV, SV>::Hybrid_data()
    : num_triangles_(0),
      current_triangle_(0),
      num_intersection_vertices_(0),
      num_shading_vertices_(0),
      triangles_(nullptr),
      intersection_vertices_(nullptr),
      shading_vertices_(nullptr) {}

template <typename IV, typename SV>
Hybrid_data<IV, SV>::~Hybrid_data() {
    memory::free_aligned(shading_vertices_);
    memory::free_aligned(intersection_vertices_);
    memory::free_aligned(triangles_);
}

template <typename IV, typename SV>
uint32_t Hybrid_data<IV, SV>::num_triangles() const {
    return num_triangles_;
}

template <typename IV, typename SV>
uint32_t Hybrid_data<IV, SV>::current_triangle() const {
    return current_triangle_;
}

template <typename IV, typename SV>
bool Hybrid_data<IV, SV>::intersect(uint32_t index, math::Ray& ray, float2& uv) const {
    uint32_t const vi = index * 3;

    IV const& a = intersection_vertices_[vi + 0];
    IV const& b = intersection_vertices_[vi + 1];
    IV const& c = intersection_vertices_[vi + 2];

    return triangle::intersect(a, b, c, ray, uv);
}

template <typename IV, typename SV>
bool Hybrid_data<IV, SV>::intersect_p(uint32_t index, math::Ray const& ray) const {
    uint32_t const vi = index * 3;

    IV const& a = intersection_vertices_[vi + 0];
    IV const& b = intersection_vertices_[vi + 1];
    IV const& c = intersection_vertices_[vi + 2];

    return triangle::intersect_p(a, b, c, ray);
}

template <typename IV, typename SV>
bool Hybrid_data<IV, SV>::intersect(FVector origin, FVector direction, FVector min_t, Vector& max_t,
                                    uint32_t index, Vector& u, Vector& v) const {
    uint32_t const vi = index * 3;

    IV const& a = intersection_vertices_[vi + 0];
    IV const& b = intersection_vertices_[vi + 1];
    IV const& c = intersection_vertices_[vi + 2];

    return triangle::intersect(origin, direction, min_t, max_t, a, b, c, u, v);
}

template <typename IV, typename SV>
bool Hybrid_data<IV, SV>::intersect(FVector origin, FVector direction, FVector min_t, Vector& max_t,
                                    uint32_t index) const {
    uint32_t const vi = index * 3;

    IV const& a = intersection_vertices_[vi + 0];
    IV const& b = intersection_vertices_[vi + 1];
    IV const& c = intersection_vertices_[vi + 2];

    return triangle::intersect(origin, direction, min_t, max_t, a, b, c);
}

template <typename IV, typename SV>
bool Hybrid_data<IV, SV>::intersect_p(FVector origin, FVector direction, FVector min_t,
                                      FVector max_t, uint32_t index) const {
    uint32_t const vi = index * 3;

    IV const& a = intersection_vertices_[vi + 0];
    IV const& b = intersection_vertices_[vi + 1];
    IV const& c = intersection_vertices_[vi + 2];

    return triangle::intersect_p(origin, direction, min_t, max_t, a, b, c);
}

template <typename IV, typename SV>
void Hybrid_data<IV, SV>::interpolate_data(uint32_t index, float2 uv, float3& n, float3& t,
                                           float2& tc) const {
    auto const& tri = triangles_[index];

    SV const& a = shading_vertices_[tri.a];
    SV const& b = shading_vertices_[tri.b];
    SV const& c = shading_vertices_[tri.c];

    triangle::interpolate_data(a, b, c, uv, n, t, tc);
}

template <typename IV, typename SV>
void Hybrid_data<IV, SV>::interpolate_data(FVector u, FVector v, uint32_t index, float3& n,
                                           float3& t, float2& tc) const {
    auto const& tri = triangles_[index];

    SV const& a = shading_vertices_[tri.a];
    SV const& b = shading_vertices_[tri.b];
    SV const& c = shading_vertices_[tri.c];

    triangle::interpolate_data(u, v, a, b, c, n, t, tc);
}

template <typename IV, typename SV>
void Hybrid_data<IV, SV>::interpolate_data(FVector u, FVector v, uint32_t index, Vector& n,
                                           Vector& t, float2& tc) const {
    auto const& tri = triangles_[index];

    SV const& a = shading_vertices_[tri.a];
    SV const& b = shading_vertices_[tri.b];
    SV const& c = shading_vertices_[tri.c];

    triangle::interpolate_data(u, v, a, b, c, n, t, tc);
}

template <typename IV, typename SV>
float2 Hybrid_data<IV, SV>::interpolate_uv(uint32_t index, float2 uv) const {
    auto const& tri = triangles_[index];

    SV const& sa = shading_vertices_[tri.a];
    SV const& sb = shading_vertices_[tri.b];
    SV const& sc = shading_vertices_[tri.c];

    return triangle::interpolate_uv(sa, sb, sc, uv);
}

template <typename IV, typename SV>
float2 Hybrid_data<IV, SV>::interpolate_uv(FVector u, FVector v, uint32_t index) const {
    auto const& tri = triangles_[index];

    SV const& sa = shading_vertices_[tri.a];
    SV const& sb = shading_vertices_[tri.b];
    SV const& sc = shading_vertices_[tri.c];

    return triangle::interpolate_uv(u, v, sa, sb, sc);
}

template <typename IV, typename SV>
float Hybrid_data<IV, SV>::bitangent_sign(uint32_t index) const {
    static float constexpr signs[2] = {1.f, -1.f};

    return signs[triangles_[index].bts];
}

template <typename IV, typename SV>
uint32_t Hybrid_data<IV, SV>::material_index(uint32_t index) const {
    return triangles_[index].material_index;
}

template <typename IV, typename SV>
float3 Hybrid_data<IV, SV>::normal(uint32_t index) const {
    uint32_t const vi = index * 3;

    IV const& a = intersection_vertices_[vi + 0];
    IV const& b = intersection_vertices_[vi + 1];
    IV const& c = intersection_vertices_[vi + 2];

    float3 const e1 = b.p - a.p;
    float3 const e2 = c.p - a.p;
    return math::normalize(math::cross(e1, e2));
}

template <typename IV, typename SV>
Vector Hybrid_data<IV, SV>::normal_v(uint32_t index) const {
    uint32_t const vi = index * 3;

    IV const& a = intersection_vertices_[vi + 0];
    IV const& b = intersection_vertices_[vi + 1];
    IV const& c = intersection_vertices_[vi + 2];

    Vector const ap = simd::load_float4(a.p.v);
    Vector const bp = simd::load_float4(b.p.v);
    Vector const cp = simd::load_float4(c.p.v);

    Vector const e1 = math::sub(bp, ap);
    Vector const e2 = math::sub(cp, ap);

    return math::normalized3(math::cross3(e1, e2));
}

template <typename IV, typename SV>
float Hybrid_data<IV, SV>::area(uint32_t index) const {
    uint32_t const vi = index * 3;

    IV const& a = intersection_vertices_[vi + 0];
    IV const& b = intersection_vertices_[vi + 1];
    IV const& c = intersection_vertices_[vi + 2];

    return triangle::area(a, b, c);
}

template <typename IV, typename SV>
float Hybrid_data<IV, SV>::area(uint32_t index, f_float3 scale) const {
    uint32_t const vi = index * 3;

    IV const& a = intersection_vertices_[vi + 0];
    IV const& b = intersection_vertices_[vi + 1];
    IV const& c = intersection_vertices_[vi + 2];

    return triangle::area(a, b, c, scale);
}

template <typename IV, typename SV>
void Hybrid_data<IV, SV>::sample(uint32_t index, float2 r2, float3& p, float2& tc) const {
    SOFT_ASSERT(index < num_triangles_);

    float2 const uv = math::sample_triangle_uniform(r2);

    uint32_t const vi = index * 3;

    IV const& ia = intersection_vertices_[vi + 0];
    IV const& ib = intersection_vertices_[vi + 1];
    IV const& ic = intersection_vertices_[vi + 2];

    triangle::interpolate_p(ia, ib, ic, uv, p);

    auto const& tri = triangles_[index];

    SV const& sa = shading_vertices_[tri.a];
    SV const& sb = shading_vertices_[tri.b];
    SV const& sc = shading_vertices_[tri.c];

    tc = triangle::interpolate_uv(sa, sb, sc, uv);
}

template <typename IV, typename SV>
void Hybrid_data<IV, SV>::allocate_triangles(uint32_t num_triangles, Vertices const& vertices) {
    uint32_t const num_intersection_vertices = num_triangles * 3;
    uint32_t const num_shading_vertices      = static_cast<uint32_t>(vertices.size());

    if (num_triangles != num_triangles_ || num_shading_vertices != num_shading_vertices_) {
        num_triangles_             = num_triangles;
        num_intersection_vertices_ = num_intersection_vertices;
        num_shading_vertices_      = num_shading_vertices;

        memory::free_aligned(shading_vertices_);
        memory::free_aligned(intersection_vertices_);
        memory::free_aligned(triangles_);

        triangles_             = memory::allocate_aligned<Index_triangle>(num_triangles);
        intersection_vertices_ = memory::allocate_aligned<IV>(num_intersection_vertices);
        shading_vertices_      = memory::allocate_aligned<SV>(num_shading_vertices);
    }

    for (uint32_t i = 0; i < num_shading_vertices; ++i) {
        shading_vertices_[i] = SV(vertices[i].n, vertices[i].t, vertices[i].uv);
    }

    current_triangle_ = 0;
}

template <typename IV, typename SV>
void Hybrid_data<IV, SV>::add_triangle(uint32_t a, uint32_t b, uint32_t c, uint32_t material_index,
                                       Vertices const& vertices) {
    uint32_t const v = current_triangle_ * 3;

    intersection_vertices_[v + 0].p = float3(vertices[a].p);
    intersection_vertices_[v + 1].p = float3(vertices[b].p);
    intersection_vertices_[v + 2].p = float3(vertices[c].p);

    float bitanget_sign = 1.f;

    if ((vertices[a].bitangent_sign < 0.f && vertices[b].bitangent_sign < 0.f) ||
        (vertices[b].bitangent_sign < 0.f && vertices[c].bitangent_sign < 0.f) ||
        (vertices[c].bitangent_sign < 0.f && vertices[a].bitangent_sign < 0.f)) {
        bitanget_sign = -1.f;
    }

    triangles_[current_triangle_] = Index_triangle(a, b, c, bitanget_sign, material_index);
    ++current_triangle_;
}

template <typename IV, typename SV>
size_t Hybrid_data<IV, SV>::num_bytes() const {
    return sizeof(*this) + num_triangles_ * sizeof(Index_triangle) +
           num_intersection_vertices_ * sizeof(IV) + num_shading_vertices_ * sizeof(SV);
}

template <typename IV, typename SV>
Hybrid_data<IV, SV>::Index_triangle::Index_triangle(uint32_t a, uint32_t b, uint32_t c,
                                                    float bitangent_sign, uint32_t material_index)
    : a(a), b(b), c(c), bts(bitangent_sign < 0.f ? 1 : 0), material_index(material_index) {}

}  // namespace scene::shape::triangle::bvh

#endif
