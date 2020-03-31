#ifndef SU_CORE_SCENE_SHAPE_TRIANGLE_BVH_INDEXED_DATA_INL
#define SU_CORE_SCENE_SHAPE_TRIANGLE_BVH_INDEXED_DATA_INL

#include "base/math/sampling.inl"
#include "base/memory/align.hpp"
#include "base/math/quaternion.inl"
#include "scene/shape/triangle/triangle_primitive_mt.inl"
#include "triangle_bvh_indexed_data.hpp"

#include "base/debug/assert.hpp"

namespace scene::shape::triangle::bvh {

template <typename SV>
Indexed_data<SV>::Indexed_data()
    : num_triangles_(0),
      num_vertices_(0),
      triangles_(nullptr),
      intersection_vertices_(nullptr),
      shading_vertices_(nullptr) {}

template <typename SV>
Indexed_data<SV>::~Indexed_data() {
    memory::free_aligned(shading_vertices_);
    memory::free_aligned(intersection_vertices_);
    memory::free_aligned(triangles_);
}

template <typename SV>
uint32_t Indexed_data<SV>::num_triangles() const {
    return num_triangles_;
}

template <typename SV>
bool Indexed_data<SV>::intersect(uint32_t index, ray& ray, float2& uv) const {
    auto const t = triangles_[index];

    float3 const a = intersection_vertices_[t.a];
    float3 const b = intersection_vertices_[t.b];
    float3 const c = intersection_vertices_[t.c];

    return triangle::intersect(a, b, c, ray, uv);
}

template <typename SV>
bool Indexed_data<SV>::intersect_p(uint32_t index, ray const& ray) const {
    auto const tri = triangles_[index];

    float3 const a = intersection_vertices_[tri.a];
    float3 const b = intersection_vertices_[tri.b];
    float3 const c = intersection_vertices_[tri.c];

    return triangle::intersect_p(a, b, c, ray);
}

template <typename SV>
bool Indexed_data<SV>::intersect(Simd3f const& origin, Simd3f const& direction, scalar const& min_t,
                                 scalar& max_t, uint32_t index, scalar& u, scalar& v) const {
    auto const tri = triangles_[index];

    float const* a = intersection_vertices_[tri.a].v;
    float const* b = intersection_vertices_[tri.b].v;
    float const* c = intersection_vertices_[tri.c].v;

    return triangle::intersect(origin, direction, min_t, max_t, a, b, c, u, v);
}

template <typename SV>
bool Indexed_data<SV>::intersect(Simd3f const& origin, Simd3f const& direction, scalar const& min_t,
                                 scalar& max_t, uint32_t index) const {
    auto const tri = triangles_[index];

    float const* a = intersection_vertices_[tri.a].v;
    float const* b = intersection_vertices_[tri.b].v;
    float const* c = intersection_vertices_[tri.c].v;

    return triangle::intersect(origin, direction, min_t, max_t, a, b, c);
}

template <typename SV>
bool Indexed_data<SV>::intersect_p(Simd3f const& origin, Simd3f const& direction,
                                   scalar const& min_t, scalar const& max_t, uint32_t index) const {
    auto const tri = triangles_[index];

    float const* a = intersection_vertices_[tri.a].v;
    float const* b = intersection_vertices_[tri.b].v;
    float const* c = intersection_vertices_[tri.c].v;

    return triangle::intersect_p(origin, direction, min_t, max_t, a, b, c);
}

template <typename SV>
float3 Indexed_data<SV>::interpolate_p(float2 uv, uint32_t index) const {
    auto const tri = triangles_[index];

    float3 const ap = intersection_vertices_[tri.a];
    float3 const bp = intersection_vertices_[tri.b];
    float3 const cp = intersection_vertices_[tri.c];

    float3 p;
    triangle::interpolate_p(ap, bp, cp, uv, p);
    return p;
}

template <typename SV>
Simd3f Indexed_data<SV>::interpolate_p(Simd3f const& u, Simd3f const& v, uint32_t index) const {
    auto const tri = triangles_[index];

    Simd3f const ap(intersection_vertices_[tri.a].v);
    Simd3f const bp(intersection_vertices_[tri.b].v);
    Simd3f const cp(intersection_vertices_[tri.c].v);

    return triangle::interpolate_p(ap, bp, cp, u, v);
}

template <typename SV>
void Indexed_data<SV>::interpolate_data(uint32_t index, float2 uv, float3& n, float3& t,
                                        float2& tc) const {
    auto const tri = triangles_[index];

    SV const& a = shading_vertices_[tri.a];
    SV const& b = shading_vertices_[tri.b];
    SV const& c = shading_vertices_[tri.c];

    triangle::interpolate_data(a, b, c, uv, n, t, tc);
}

template <typename SV>
void Indexed_data<SV>::interpolate_data(Simd3f const& u, Simd3f const& v, uint32_t index, Simd3f& n,
                                        Simd3f& t, float2& tc) const {
    auto const tri = triangles_[index];

    SV const& a = shading_vertices_[tri.a];
    SV const& b = shading_vertices_[tri.b];
    SV const& c = shading_vertices_[tri.c];

    triangle::interpolate_data(u, v, a, b, c, n, t, tc);
}

template <typename SV>
Simd3f Indexed_data<SV>::interpolate_shading_normal(Simd3f const& u, Simd3f const& v,
                                                    uint32_t index) const {
    auto const tri = triangles_[index];

    SV const& a = shading_vertices_[tri.a];
    SV const& b = shading_vertices_[tri.b];
    SV const& c = shading_vertices_[tri.c];

    return triangle::interpolate_normal(u, v, a, b, c);
}

template <typename SV>
float2 Indexed_data<SV>::interpolate_uv(uint32_t index, float2 uv) const {
    auto const tri = triangles_[index];

    SV const& a = shading_vertices_[tri.a];
    SV const& b = shading_vertices_[tri.b];
    SV const& c = shading_vertices_[tri.c];

    return triangle::interpolate_uv(a, b, c, uv);
}

template <typename SV>
float2 Indexed_data<SV>::interpolate_uv(Simd3f const& u, Simd3f const& v, uint32_t index) const {
    auto const tri = triangles_[index];

    SV const& a = shading_vertices_[tri.a];
    SV const& b = shading_vertices_[tri.b];
    SV const& c = shading_vertices_[tri.c];

    return triangle::interpolate_uv(u, v, a, b, c);
}

template <typename SV>
float Indexed_data<SV>::bitangent_sign(uint32_t index) const {
    static float constexpr signs[2] = {1.f, -1.f};

    return signs[triangles_[index].bts];
}

template <typename SV>
uint32_t Indexed_data<SV>::part(uint32_t index) const {
    return triangles_[index].part;
}

template <typename SV>
float3 Indexed_data<SV>::normal(uint32_t index) const {
    auto const tri = triangles_[index];

    float3 const a = intersection_vertices_[tri.a];
    float3 const b = intersection_vertices_[tri.b];
    float3 const c = intersection_vertices_[tri.c];

    float3 const e1 = b - a;
    float3 const e2 = c - a;

    return normalize(cross(e1, e2));
}

template <typename SV>
Simd3f Indexed_data<SV>::normal_v(uint32_t index) const {
    auto const tri = triangles_[index];

    Simd3f const ap(intersection_vertices_[tri.a].v);
    Simd3f const bp(intersection_vertices_[tri.b].v);
    Simd3f const cp(intersection_vertices_[tri.c].v);

    Simd3f const e1 = bp - ap;
    Simd3f const e2 = cp - ap;

    return normalize(cross(e1, e2));
}

template <typename SV>
float Indexed_data<SV>::area(uint32_t index) const {
    auto const tri = triangles_[index];

    float3 const a = intersection_vertices_[tri.a];
    float3 const b = intersection_vertices_[tri.b];
    float3 const c = intersection_vertices_[tri.c];

    return triangle::area(a, b, c);
}

template <typename SV>
float Indexed_data<SV>::area(uint32_t index, float3 const& scale) const {
    auto const tri = triangles_[index];

    float3 const a = intersection_vertices_[tri.a];
    float3 const b = intersection_vertices_[tri.b];
    float3 const c = intersection_vertices_[tri.c];

    return triangle::area(a, b, c, scale);
}

template <typename SV>
float3 Indexed_data<SV>::center(uint32_t index) const {
    auto const tri = triangles_[index];

    float3 const a = intersection_vertices_[tri.a];
    float3 const b = intersection_vertices_[tri.b];
    float3 const c = intersection_vertices_[tri.c];

    return (a + b + c) / 3.f;
}

template <typename SV>
void Indexed_data<SV>::sample(uint32_t index, float2 r2, float3& p, float2& tc) const {
    SOFT_ASSERT(index < num_triangles_);

    float2 const uv = sample_triangle_uniform(r2);

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
void Indexed_data<SV>::allocate_triangles(uint32_t num_triangles, Vertex_stream const& vertices) {
    uint32_t const num_vertices = vertices.num_vertices();
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
        intersection_vertices_[i] = vertices.p(i);

        shading_vertices_[i] = SV(vertices.n(i), vertices.t(i), vertices.uv(i));
    }
}

template <typename SV>
void Indexed_data<SV>::add_triangle(uint32_t a, uint32_t b, uint32_t c, uint32_t part,
                                    Vertex_stream const& vertices, uint32_t current_triangle) {
    uint8_t bitanget_sign = 0;

    uint8_t const abts = vertices.bitangent_sign(a);
    uint8_t const bbts = vertices.bitangent_sign(b);
    uint8_t const cbts = vertices.bitangent_sign(c);

    if ((abts == 1 && bbts == 1) || (bbts == 1 && cbts == 1) || (cbts == 1 && abts == 1)) {
        bitanget_sign = 1;
    }

    triangles_[current_triangle] = Index_triangle(a, b, c, bitanget_sign, part);
}

template <typename SV>
size_t Indexed_data<SV>::num_bytes() const {
    return sizeof(*this) + num_triangles_ * sizeof(Index_triangle) +
           num_vertices_ * (sizeof(float3) + sizeof(SV));
}

template <typename SV>
Indexed_data<SV>::Index_triangle::Index_triangle(uint32_t a, uint32_t b, uint32_t c,
                                                 uint8_t bitangent_sign, uint32_t part)
    : a(a), b(b), c(c), bts(bitangent_sign), part(part) {}

Indexed_data1::Indexed_data1()
    : num_triangles_(0),
      num_vertices_(0),
      triangles_(nullptr),
      intersection_vertices_(nullptr),
      tangent_frames_(nullptr),
      uvs_(nullptr) {}

Indexed_data1::~Indexed_data1() {
    memory::free_aligned(uvs_);
    memory::free_aligned(tangent_frames_);
    memory::free_aligned(intersection_vertices_);
    memory::free_aligned(triangles_);
}

uint32_t Indexed_data1::num_triangles() const {
    return num_triangles_;
}

bool Indexed_data1::intersect(uint32_t index, ray& ray, float2& uv) const {
    auto const t = triangles_[index];

    float3 const a = intersection_vertices_[t.a];
    float3 const b = intersection_vertices_[t.b];
    float3 const c = intersection_vertices_[t.c];

    return triangle::intersect(a, b, c, ray, uv);
}

bool Indexed_data1::intersect_p(uint32_t index, ray const& ray) const {
    auto const tri = triangles_[index];

    float3 const a = intersection_vertices_[tri.a];
    float3 const b = intersection_vertices_[tri.b];
    float3 const c = intersection_vertices_[tri.c];

    return triangle::intersect_p(a, b, c, ray);
}

bool Indexed_data1::intersect(Simd3f const& origin, Simd3f const& direction, scalar const& min_t,
                                 scalar& max_t, uint32_t index, scalar& u, scalar& v) const {
    auto const tri = triangles_[index];

    float const* a = intersection_vertices_[tri.a].v;
    float const* b = intersection_vertices_[tri.b].v;
    float const* c = intersection_vertices_[tri.c].v;

    return triangle::intersect(origin, direction, min_t, max_t, a, b, c, u, v);
}

bool Indexed_data1::intersect(Simd3f const& origin, Simd3f const& direction, scalar const& min_t,
                                 scalar& max_t, uint32_t index) const {
    auto const tri = triangles_[index];

    float const* a = intersection_vertices_[tri.a].v;
    float const* b = intersection_vertices_[tri.b].v;
    float const* c = intersection_vertices_[tri.c].v;

    return triangle::intersect(origin, direction, min_t, max_t, a, b, c);
}

bool Indexed_data1::intersect_p(Simd3f const& origin, Simd3f const& direction,
                                   scalar const& min_t, scalar const& max_t, uint32_t index) const {
    auto const tri = triangles_[index];

    float const* a = intersection_vertices_[tri.a].v;
    float const* b = intersection_vertices_[tri.b].v;
    float const* c = intersection_vertices_[tri.c].v;

    return triangle::intersect_p(origin, direction, min_t, max_t, a, b, c);
}

float3 Indexed_data1::interpolate_p(float2 uv, uint32_t index) const {
    auto const tri = triangles_[index];

    float3 const ap = intersection_vertices_[tri.a];
    float3 const bp = intersection_vertices_[tri.b];
    float3 const cp = intersection_vertices_[tri.c];

    float3 p;
    triangle::interpolate_p(ap, bp, cp, uv, p);
    return p;
}

Simd3f Indexed_data1::interpolate_p(Simd3f const& u, Simd3f const& v, uint32_t index) const {
    auto const tri = triangles_[index];

    Simd3f const ap(intersection_vertices_[tri.a].v);
    Simd3f const bp(intersection_vertices_[tri.b].v);
    Simd3f const cp(intersection_vertices_[tri.c].v);

    return triangle::interpolate_p(ap, bp, cp, u, v);
}

void Indexed_data1::interpolate_data(uint32_t index, float2 uv, float3& n, float3& t,
                                        float2& tc) const {
    auto const tri = triangles_[index];

    float const w = 1.f - uv[0] - uv[1];

    {
        Quaternion const a = tangent_frames_[tri.a];
        Quaternion const b = tangent_frames_[tri.b];
        Quaternion const c = tangent_frames_[tri.c];

        Quaternion const q = w * a + uv[0] * b + uv[1] * c;

        float3x3 const tf = quaternion::create_matrix3x3(q);

        n = tf.r[2];
        t = tf.r[0];
    }

    {
        float2 const a = uvs_[tri.a];
        float2 const b = uvs_[tri.b];
        float2 const c = uvs_[tri.c];

        tc = w * a + uv[0] * b + uv[1] * c;
    }
}

//void toTangentFrame(const highp vec4 q, out highp vec3 n) {
//    n = vec3( 0.0,  0.0,  1.0) +
//        vec3( 2.0, -2.0, -2.0) * q.x * q.zwx +
//        vec3( 2.0,  2.0, -2.0) * q.y * q.wzy;
//}

///**
// * Extracts the normal and tangent vectors of the tangent frame encoded in the
// * specified quaternion.
// */
//void toTangentFrame(const highp vec4 q, out highp vec3 n, out highp vec3 t) {
//    toTangentFrame(q, n);
//    t = vec3( 1.0,  0.0,  0.0) +
//        vec3(-2.0,  2.0, -2.0) * q.y * q.yxw +
//        vec3(-2.0,  2.0,  2.0) * q.z * q.zwx;
//}

static inline void toTangentFrame(Quaternion const& q, float3& n) {
    n = float3( 0.0,  0.0,  1.0) +
        q[0] * float3( 2.0, -2.0, -2.0) * float3(q[2], q[3], q[0]) +
        q[1] * float3( 2.0,  2.0, -2.0) * float3(q[3], q[2], q[1]);
}

static inline void toTangentFrame(Quaternion const& q, float3& n, float3& t) {
    toTangentFrame(q, n);
    t = float3( 1.0,  0.0,  0.0) +
        q[1] * float3(-2.0,  2.0, -2.0)  * float3(q[1], q[0], q[3]) +
        q[2] * float3(-2.0,  2.0,  2.0) * float3(q[2], q[3], q[0]);
}

void Indexed_data1::interpolate_data(Simd3f const& u, Simd3f const& v, uint32_t index, Simd3f& n,
                                        Simd3f& t, float2& tc) const {
    auto const tri = triangles_[index];

    float2 const uv(u.x(), v.x());

    float const w = 1.f - uv[0] - uv[1];

    {
        Quaternion const a = tangent_frames_[tri.a];
        Quaternion const b = tangent_frames_[tri.b];
        Quaternion const c = tangent_frames_[tri.c];

        Quaternion const q = w * a + uv[0] * b + uv[1] * c;

        float3x3 const tf = quaternion::create_matrix3x3(q);

        n = Simd3f(tf.r[2].v);
        t = Simd3f(tf.r[0].v);
    }

    {
        float2 const a = uvs_[tri.a];
        float2 const b = uvs_[tri.b];
        float2 const c = uvs_[tri.c];

        tc = w * a + uv[0] * b + uv[1] * c;
    }
}

void Indexed_data1::interpolate_data(Simd3f const& u, Simd3f const& v, uint32_t index, Simd3f& t, Simd3f& b, Simd3f& n,
                      float2& tc) const {
    auto const tri = triangles_[index];

    float2 const uv(u.x(), v.x());

    float const w = 1.f - uv[0] - uv[1];

    {
        Quaternion const tsa = tangent_frames_[tri.a];
        Quaternion const tsb = tangent_frames_[tri.b];
        Quaternion const tsc = tangent_frames_[tri.c];

        Quaternion const q = w * tsa + uv[0] * tsb + uv[1] * tsc;

        float3x3 const tf = quaternion::create_matrix3x3(q);

        t = Simd3f(tf.r[0].v);
        b = Simd3f(tf.r[1].v);
        n = Simd3f(tf.r[2].v);

    }

    {
        float2 const uva = uvs_[tri.a];
        float2 const uvb = uvs_[tri.b];
        float2 const uvc = uvs_[tri.c];

        tc = w * uva + uv[0] * uvb + uv[1] * uvc;
    }
}

Simd3f Indexed_data1::interpolate_shading_normal(Simd3f const& u, Simd3f const& v,
                                                    uint32_t index) const {
    auto const tri = triangles_[index];

    Simd3f const w = simd::One - u - v;

    Simd3f va = w * Simd3f(tangent_frames_[tri.a].v);
    Simd3f vb = u * Simd3f(tangent_frames_[tri.b].v);

    va = va + vb;

    Simd3f vc = v * Simd3f(tangent_frames_[tri.c].v);
    Simd3f v0 = va + vc;

    return normalize(v0);
}

float2 Indexed_data1::interpolate_uv(uint32_t index, float2 uv) const {
    auto const tri = triangles_[index];

    float2 const a = uvs_[tri.a];
    float2 const b = uvs_[tri.b];
    float2 const c = uvs_[tri.c];

    float const w = 1.f - uv[0] - uv[1];

    return w * a + uv[0] * b + uv[1] * c;
}

float2 Indexed_data1::interpolate_uv(Simd3f const& u, Simd3f const& v, uint32_t index) const {
    auto const tri = triangles_[index];

    float2 const a = uvs_[tri.a];
    float2 const b = uvs_[tri.b];
    float2 const c = uvs_[tri.c];

    Simd3f const w = simd::One - u - v;

    Simd3f va(a[0], a[1], 0.f);
    Simd3f vb(b[0], b[1], 0.f);

    va = w * va;
    vb = u * vb;
    va = va + vb;

    Simd3f vc(c[0], c[1], 0.f);

    vc = v * vc;

    Simd3f const uv = va + vc;

    return float3(uv).xy();
}

float Indexed_data1::bitangent_sign(uint32_t /*index*/) const {
    return 1.f;
}

uint32_t Indexed_data1::part(uint32_t index) const {
    return triangles_[index].part;
}

float3 Indexed_data1::normal(uint32_t index) const {
    auto const tri = triangles_[index];

    float3 const a = intersection_vertices_[tri.a];
    float3 const b = intersection_vertices_[tri.b];
    float3 const c = intersection_vertices_[tri.c];

    float3 const e1 = b - a;
    float3 const e2 = c - a;

    return normalize(cross(e1, e2));
}

Simd3f Indexed_data1::normal_v(uint32_t index) const {
    auto const tri = triangles_[index];

    Simd3f const ap(intersection_vertices_[tri.a].v);
    Simd3f const bp(intersection_vertices_[tri.b].v);
    Simd3f const cp(intersection_vertices_[tri.c].v);

    Simd3f const e1 = bp - ap;
    Simd3f const e2 = cp - ap;

    return normalize(cross(e1, e2));
}

float Indexed_data1::area(uint32_t index) const {
    auto const tri = triangles_[index];

    float3 const a = intersection_vertices_[tri.a];
    float3 const b = intersection_vertices_[tri.b];
    float3 const c = intersection_vertices_[tri.c];

    return triangle::area(a, b, c);
}

float Indexed_data1::area(uint32_t index, float3 const& scale) const {
    auto const tri = triangles_[index];

    float3 const a = intersection_vertices_[tri.a];
    float3 const b = intersection_vertices_[tri.b];
    float3 const c = intersection_vertices_[tri.c];

    return triangle::area(a, b, c, scale);
}

float3 Indexed_data1::center(uint32_t index) const {
    auto const tri = triangles_[index];

    float3 const a = intersection_vertices_[tri.a];
    float3 const b = intersection_vertices_[tri.b];
    float3 const c = intersection_vertices_[tri.c];

    return (a + b + c) / 3.f;
}

void Indexed_data1::sample(uint32_t index, float2 r2, float3& p, float2& tc) const {
    SOFT_ASSERT(index < num_triangles_);

    float2 const uv = sample_triangle_uniform(r2);

    auto const tri = triangles_[index];

    float3 const ia = intersection_vertices_[tri.a];
    float3 const ib = intersection_vertices_[tri.b];
    float3 const ic = intersection_vertices_[tri.c];

    triangle::interpolate_p(ia, ib, ic, uv, p);

    float2 const a = uvs_[tri.a];
    float2 const b = uvs_[tri.b];
    float2 const c = uvs_[tri.c];

    float const w = 1.f - uv[0] - uv[1];

    tc = float2(w * a[0] + uv[0] * b[0] + uv[1] * c[0],
                  w * a[1] + uv[0] * b[1] + uv[1] * c[1]);
}

void Indexed_data1::allocate_triangles(uint32_t num_triangles, Vertex_stream const& vertices) {
    uint32_t const num_vertices = vertices.num_vertices();
    if (num_triangles != num_triangles_ || num_vertices != num_vertices_) {
        num_triangles_ = num_triangles;
        num_vertices_  = num_vertices;

        memory::free_aligned(uvs_);
        memory::free_aligned(tangent_frames_);
        memory::free_aligned(intersection_vertices_);
        memory::free_aligned(triangles_);

        triangles_             = memory::allocate_aligned<Index_triangle>(num_triangles);
        intersection_vertices_ = memory::allocate_aligned<float3>(num_vertices);
        tangent_frames_      = memory::allocate_aligned<float4>(num_vertices);
        uvs_      = memory::allocate_aligned<float2>(num_vertices);
    }

    for (uint32_t i = 0; i < num_vertices; ++i) {
        intersection_vertices_[i] = vertices.p(i);

        float3 const t = vertices.t(i);
        float3 const n = vertices.n(i);

     //   float3 const b0 = (vertices.bitangent_sign(i) ? 1.f : -1.f) * cross(t, n);

        float3 const b = cross(n, t);


        Quaternion q = quaternion::create(float3x3(t, b, n));

        if (q[3] < 0.f && vertices.bitangent_sign(i) == 0) {
            q[3] = -q[3];
        }

        tangent_frames_[i] = q;

        uvs_[i] = vertices.uv(i);
    }
}

void Indexed_data1::add_triangle(uint32_t a, uint32_t b, uint32_t c, uint32_t part,
                                    Vertex_stream const& /*vertices*/, uint32_t current_triangle) {
    triangles_[current_triangle] = Index_triangle(a, b, c, part);
}

size_t Indexed_data1::num_bytes() const {
    return sizeof(*this) + num_triangles_ * sizeof(Index_triangle) +
           num_vertices_ * (sizeof(float3) + sizeof(float4) + sizeof(float2));
}

Indexed_data1::Index_triangle::Index_triangle(uint32_t a, uint32_t b, uint32_t c,
                                              uint32_t part)
    : a(a), b(b), c(c), part(part) {}

}  // namespace scene::shape::triangle::bvh

#endif
