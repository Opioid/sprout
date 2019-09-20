#ifndef SU_CORE_SCENE_SHAPE_TRIANGLE_PRIMITIVE_MT_INL
#define SU_CORE_SCENE_SHAPE_TRIANGLE_PRIMITIVE_MT_INL

#include "base/encoding/encoding.inl"
#include "base/math/ray.hpp"
#include "base/math/simd_vector.inl"
#include "triangle_primitive_mt.hpp"

namespace scene::shape::triangle {

inline Triangle_MT::Triangle_MT(const shape::Vertex& a, const shape::Vertex& b,
                                const shape::Vertex& c, float bitangent_sign,
                                uint32_t material_index) noexcept
    : a(a), b(b), c(c), bitangent_sign(bitangent_sign), material_index(material_index) {}

inline Triangle_MT::Vertex::Vertex(const shape::Vertex& v) noexcept
    : p(v.p), n(v.n), t(v.t), uv(v.uv) {}

inline bool Triangle_MT::intersect(ray& ray, float2& uv) const noexcept {
    float3 e1 = b.p - a.p;
    float3 e2 = c.p - a.p;

    float3 pvec = cross(ray.direction, e2);

    float det     = dot(e1, pvec);
    float inv_det = 1.f / det;

    float3 tvec = ray.origin - a.p;
    float  u    = dot(tvec, pvec) * inv_det;

    if (u < 0.f || u > 1.f) {
        return false;
    }

    float3 qvec = cross(tvec, e1);
    float  v    = dot(ray.direction, qvec) * inv_det;

    if (v < 0.f || u + v > 1.f) {
        return false;
    }

    float hit_t = dot(e2, qvec) * inv_det;

    if (hit_t > ray.min_t && hit_t < ray.max_t) {
        ray.max_t = hit_t;
        uv[0]     = u;
        uv[1]     = v;
        return true;
    }

    return false;
}

inline bool Triangle_MT::intersect_p(ray const& ray) const noexcept {
    float3 e1 = b.p - a.p;
    float3 e2 = c.p - a.p;

    float3 pvec = cross(ray.direction, e2);

    float det     = dot(e1, pvec);
    float inv_det = 1.f / det;

    float3 tvec = ray.origin - a.p;
    float  u    = dot(tvec, pvec) * inv_det;

    if (u < 0.f || u > 1.f) {
        return false;
    }

    float3 qvec = cross(tvec, e1);
    float  v    = dot(ray.direction, qvec) * inv_det;

    if (v < 0.f || u + v > 1.f) {
        return false;
    }

    float hit_t = dot(e2, qvec) * inv_det;

    if (hit_t > ray.min_t && hit_t < ray.max_t) {
        return true;
    }

    return false;
}

inline void Triangle_MT::interpolate(float2 uv, float3& p, float3& n, float2& tc) const noexcept {
    float w = 1.f - uv[0] - uv[1];

    p  = w * a.p + uv[0] * b.p + uv[1] * c.p;
    n  = normalize(w * a.n + uv[0] * b.n + uv[1] * c.n);
    tc = w * a.uv + uv[0] * b.uv + uv[1] * c.uv;
}

inline void Triangle_MT::interpolate(float2 uv, float3& p, float2& tc) const noexcept {
    float w = 1.f - uv[0] - uv[1];

    p  = w * a.p + uv[0] * b.p + uv[1] * c.p;
    tc = w * a.uv + uv[0] * b.uv + uv[1] * c.uv;
}

inline void Triangle_MT::interpolate(float2 uv, float3& p) const noexcept {
    float w = 1.f - uv[0] - uv[1];

    p = w * a.p + uv[0] * b.p + uv[1] * c.p;
}

inline void Triangle_MT::interpolate_data(float2 uv, float3& n, float3& t, float2& tc) const
    noexcept {
    float w = 1.f - uv[0] - uv[1];

    n  = normalize(w * a.n + uv[0] * b.n + uv[1] * c.n);
    t  = normalize(w * a.t + uv[0] * b.t + uv[1] * c.t);
    tc = w * a.uv + uv[0] * b.uv + uv[1] * c.uv;
}

inline float2 Triangle_MT::interpolate_uv(float2 uv) const noexcept {
    float w = 1.f - uv[0] - uv[1];

    return w * a.uv + uv[0] * b.uv + uv[1] * c.uv;
}

inline float3 Triangle_MT::normal() const noexcept {
    float3 e1 = b.p - a.p;
    float3 e2 = c.p - a.p;

    return normalize(cross(e1, e2));
}

inline float Triangle_MT::area() const noexcept {
    return 0.5f * length(cross(b.p - a.p, c.p - a.p));
}

inline float Triangle_MT::area(float3 const& scale) const noexcept {
    float3 sa = scale * a.p;
    float3 sb = scale * b.p;
    float3 sc = scale * c.p;
    return 0.5f * length(cross(sb - sa, sc - sa));
}

static inline bool intersect(float3 const& a, float3 const& b, float3 const& c, ray& ray,
                             float2& uv) noexcept {
    float3 e1 = b - a;
    float3 e2 = c - a;

    float3 pvec = cross(ray.direction, e2);

    float det     = dot(e1, pvec);
    float inv_det = 1.f / det;

    float3 tvec = ray.origin - a;
    float  u    = dot(tvec, pvec) * inv_det;

    //	if (u < 0.f || u > 1.f) {
    //		return false;
    //	}

    float3 qvec = cross(tvec, e1);
    float  v    = dot(ray.direction, qvec) * inv_det;

    //	if (v < 0.f || u + v > 1.f) {
    //		return false;
    //	}

    float hit_t = dot(e2, qvec) * inv_det;

    //	if (hit_t > ray.min_t && hit_t < ray.max_t) {
    //		ray.max_t = hit_t;
    //		uv[0] = u;
    //		uv[1] = v;
    //		return true;
    //	}

    //	return false;

    uint8_t ca = uint8_t(u > 0.f);
    uint8_t cb = uint8_t(u < 1.f);
    uint8_t cc = uint8_t(v > 0.f);
    uint8_t cd = uint8_t(u + v < 1.f);
    uint8_t ce = uint8_t(hit_t > ray.min_t);
    uint8_t cf = uint8_t(hit_t < ray.max_t);

    if (0 != (ca & cb & cc & cd & ce & cf)) {
        ray.max_t = hit_t;
        uv[0]     = u;
        uv[1]     = v;
        return true;
    }

    return false;
}

static inline bool intersect(FVector origin, FVector direction, FVector min_t, Vector& max_t,
                             float const* a, float const* b, float const* c, Vector& u_out,
                             Vector& v_out) noexcept {
    using namespace math;

    Vector ap = simd::load_float4(a);
    Vector bp = simd::load_float4(b);
    Vector cp = simd::load_float4(c);

    Vector e1   = sub(bp, ap);
    Vector e2   = sub(cp, ap);
    Vector tvec = sub(origin, ap);

    Vector pvec = cross3(direction, e2);
    Vector qvec = cross3(tvec, e1);

    Vector e1_d_pv = dot3_1(e1, pvec);
    Vector tv_d_pv = dot3_1(tvec, pvec);
    Vector di_d_qv = dot3_1(direction, qvec);
    Vector e2_d_qv = dot3_1(e2, qvec);

    Vector inv_det = rcp1(e1_d_pv);

    Vector u     = mul1(tv_d_pv, inv_det);
    Vector v     = mul1(di_d_qv, inv_det);
    Vector hit_t = mul1(e2_d_qv, inv_det);

    Vector uv = add1(u, v);

    if (0 != (_mm_ucomige_ss(u, simd::Zero) & _mm_ucomige_ss(simd::One, u) &
              _mm_ucomige_ss(v, simd::Zero) & _mm_ucomige_ss(simd::One, uv) &
              _mm_ucomige_ss(hit_t, min_t) & _mm_ucomige_ss(max_t, hit_t))) {
        max_t = hit_t;
        u_out = u;
        v_out = v;
        return true;
    }

    return false;
}

static inline bool intersect(FVector origin, FVector direction, FVector min_t, Vector& max_t,
                             float const* a, float const* b, float const* c) noexcept {
    using namespace math;

    Vector ap = simd::load_float4(a);
    Vector bp = simd::load_float4(b);
    Vector cp = simd::load_float4(c);

    Vector e1   = sub(bp, ap);
    Vector e2   = sub(cp, ap);
    Vector tvec = sub(origin, ap);

    Vector pvec = cross3(direction, e2);
    Vector qvec = cross3(tvec, e1);

    Vector e1_d_pv = dot3_1(e1, pvec);
    Vector tv_d_pv = dot3_1(tvec, pvec);
    Vector di_d_qv = dot3_1(direction, qvec);
    Vector e2_d_qv = dot3_1(e2, qvec);

    Vector inv_det = rcp1(e1_d_pv);

    Vector u     = mul1(tv_d_pv, inv_det);
    Vector v     = mul1(di_d_qv, inv_det);
    Vector hit_t = mul1(e2_d_qv, inv_det);

    Vector uv = add1(u, v);

    if (0 != (_mm_ucomige_ss(u, simd::Zero) & _mm_ucomige_ss(simd::One, u) &
              _mm_ucomige_ss(v, simd::Zero) & _mm_ucomige_ss(simd::One, uv) &
              _mm_ucomige_ss(hit_t, min_t) & _mm_ucomige_ss(max_t, hit_t))) {
        max_t = hit_t;
        return true;
    }

    return false;
}

static inline bool intersect_p(float3 const& a, float3 const& b, float3 const& c,
                               ray const& ray) noexcept {
    // Implementation A
    /*	float3 e1 = b.p - a.p;
            float3 e2 = c.p - a.p;

            float3 pvec = cross(ray.direction, e2);

            float det = dot(e1, pvec);
            float inv_det = 1.f / det;

            float3 tvec = ray.origin - a.p;
            float u = dot(tvec, pvec) * inv_det;

            if (u < 0.f || u > 1.f) {
                    return false;
            }

            float3 qvec = cross(tvec, e1);
            float v = dot(ray.direction, qvec) * inv_det;

            if (v < 0.f || u + v > 1.f) {
                    return false;
            }

            float hit_t = dot(e2, qvec) * inv_det;

            if (hit_t > ray.min_t && hit_t < ray.max_t) {
                    return true;
            }

            return false;
    */
    // Implementation B

    float3 e1 = b - a;
    float3 e2 = c - a;

    float3 pvec = cross(ray.direction, e2);

    float det     = dot(e1, pvec);
    float inv_det = 1.f / det;

    float3 tvec = ray.origin - a;
    float  u    = dot(tvec, pvec) * inv_det;

    float3 qvec = cross(tvec, e1);
    float  v    = dot(ray.direction, qvec) * inv_det;

    float hit_t = dot(e2, qvec) * inv_det;

    uint8_t ca = uint8_t(u > 0.f);
    uint8_t cb = uint8_t(u < 1.f);
    uint8_t cc = uint8_t(v > 0.f);
    uint8_t cd = uint8_t(u + v < 1.f);
    uint8_t ce = uint8_t(hit_t > ray.min_t);
    uint8_t cf = uint8_t(hit_t < ray.max_t);

    return 0 != (ca & cb & cc & cd & ce & cf);
}

static inline bool intersect_p(FVector origin, FVector direction, FVector min_t, FVector max_t,
                               float const* a, float const* b, float const* c) noexcept {
    // Implementation C
    using namespace math;

    Vector ap = simd::load_float4(a);
    Vector bp = simd::load_float4(b);
    Vector cp = simd::load_float4(c);

    Vector e1   = sub(bp, ap);
    Vector e2   = sub(cp, ap);
    Vector tvec = sub(origin, ap);

    Vector pvec = cross3(direction, e2);
    Vector qvec = cross3(tvec, e1);

    Vector e1_d_pv = dot3_1(e1, pvec);
    Vector tv_d_pv = dot3_1(tvec, pvec);
    Vector di_d_qv = dot3_1(direction, qvec);
    Vector e2_d_qv = dot3_1(e2, qvec);

    Vector inv_det = rcp1(e1_d_pv);

    Vector u     = mul1(tv_d_pv, inv_det);
    Vector v     = mul1(di_d_qv, inv_det);
    Vector hit_t = mul1(e2_d_qv, inv_det);

    Vector uv = add1(u, v);

    return 0 != (_mm_ucomige_ss(u, simd::Zero) & _mm_ucomige_ss(simd::One, u) &
                 _mm_ucomige_ss(v, simd::Zero) & _mm_ucomige_ss(simd::One, uv) &
                 _mm_ucomige_ss(hit_t, min_t) & _mm_ucomige_ss(max_t, hit_t));
}

static inline Simd3f interpolate_p(Simd3f const& a, Simd3f const& b, Simd3f const& c,
                                   Simd3f const& u, Simd3f const& v) noexcept {
    Simd3f const w = simd::One - u - v;

    return w * a + u * b + v * c;
}

static inline void interpolate_p(float3 const& a, float3 const& b, float3 const& c, float2 uv,
                                 float3& p) noexcept {
    float const w = 1.f - uv[0] - uv[1];

    p = w * a + uv[0] * b + uv[1] * c;
}

static inline float area(float3 const& a, float3 const& b, float3 const& c) noexcept {
    return 0.5f * length(cross(b - a, c - a));
}

static inline float area(float3 const& a, float3 const& b, float3 const& c,
                         float3 const& scale) noexcept {
    float3 const sa = scale * a;
    float3 const sb = scale * b;
    float3 const sc = scale * c;
    return 0.5f * length(cross(sb - sa, sc - sa));
}

static inline float2 interpolate_uv(const Shading_vertex_MT& a, const Shading_vertex_MT& b,
                                    const Shading_vertex_MT& c, float2 uv) noexcept {
    float const w = 1.f - uv[0] - uv[1];

    return w * a.uv + uv[0] * b.uv + uv[1] * c.uv;
}

static inline void interpolate_data(const Shading_vertex_MT& a, const Shading_vertex_MT& b,
                                    const Shading_vertex_MT& c, float2 uv, float3& n, float3& t,
                                    float2& tc) noexcept {
    float const w = 1.f - uv[0] - uv[1];

    n  = normalize(w * a.n + uv[0] * b.n + uv[1] * c.n);
    t  = normalize(w * a.t + uv[0] * b.t + uv[1] * c.t);
    tc = w * a.uv + uv[0] * b.uv + uv[1] * c.uv;
}

inline Shading_vertex_MTC::Shading_vertex_MTC(float const* n, float const* t, float2 uv) noexcept
    : n_u(n, uv[0]), t_v(t, uv[1]) {
    // Not too happy about handling degenerate tangents here (only one very special case even)
    if (0.f == t[0] && 0.f == t[1] && 0.f == t[2]) {
        t_v = float4(math::tangent(n_u.xyz()), uv[1]);
    }
}

static inline float2 interpolate_uv(const Shading_vertex_MTC& a, const Shading_vertex_MTC& b,
                                    const Shading_vertex_MTC& c, float2 uv) noexcept {
    float const w = 1.f - uv[0] - uv[1];

    return float2(w * a.n_u[3] + uv[0] * b.n_u[3] + uv[1] * c.n_u[3],
                  w * a.t_v[3] + uv[0] * b.t_v[3] + uv[1] * c.t_v[3]);
}

static inline float2 interpolate_uv(FVector u, FVector v, const Shading_vertex_MTC& a,
                                    const Shading_vertex_MTC& b,
                                    const Shading_vertex_MTC& c) noexcept {
    Vector const w = math::sub(math::sub(simd::One, u), v);

    float3 const auv(a.n_u[3], a.t_v[3], 0.f);
    Vector       va = simd::load_float4(auv.v);

    float3 const buv(b.n_u[3], b.t_v[3], 0.f);
    Vector       vb = simd::load_float4(buv.v);

    va = math::mul(w, va);
    vb = math::mul(u, vb);
    va = math::add(va, vb);

    float3 const cuv(c.n_u[3], c.t_v[3], 0.f);
    Vector       vc = simd::load_float4(cuv.v);

    vc        = math::mul(v, vc);
    Vector uv = math::add(va, vc);

    float3 r;
    simd::store_float4(r.v, uv);
    return r.xy();
}

static inline void interpolate_data(const Shading_vertex_MTC& a, const Shading_vertex_MTC& b,
                                    const Shading_vertex_MTC& c, float2 uv, float3& n, float3& t,
                                    float2& tc) noexcept {
    float const w = 1.f - uv[0] - uv[1];

    float4 const n_u = w * a.n_u + uv[0] * b.n_u + uv[1] * c.n_u;
    float4 const t_v = w * a.t_v + uv[0] * b.t_v + uv[1] * c.t_v;

    n = normalize(n_u.xyz());
    t = normalize(t_v.xyz());

    tc = float2(n_u[3], t_v[3]);
}

static inline void interpolate_data(FVector u, FVector v, const Shading_vertex_MTC& a,
                                    const Shading_vertex_MTC& b, const Shading_vertex_MTC& c,
                                    float3& n, float3& t, float2& tc) noexcept {
    Vector const w = math::sub(math::sub(simd::One, u), v);

    Vector va = math::mul(w, simd::load_float4(a.n_u.v));
    Vector vb = math::mul(u, simd::load_float4(b.n_u.v));
    va        = math::add(va, vb);
    Vector vc = math::mul(v, simd::load_float4(c.n_u.v));
    Vector v0 = math::add(va, vc);

    Vector vn = math::normalized3(v0);
    simd::store_float4(n.v, vn);

    va        = math::mul(w, simd::load_float4(a.t_v.v));
    vb        = math::mul(u, simd::load_float4(b.t_v.v));
    va        = math::add(va, vb);
    vc        = math::mul(v, simd::load_float4(c.t_v.v));
    Vector v1 = math::add(va, vc);

    Vector vt = math::normalized3(v1);
    simd::store_float4(t.v, vt);

    v0 = SU_MUX_HIGH(v0, v1);
    float4 r;
    simd::store_float4(r.v, v0);
    tc[0] = r[3];
    tc[1] = r[1];
}

static inline void interpolate_data(FVector u, FVector v, const Shading_vertex_MTC& a,
                                    const Shading_vertex_MTC& b, const Shading_vertex_MTC& c,
                                    Vector& n, Vector& t, float2& tc) noexcept {
    Vector const w = math::sub(math::sub(simd::One, u), v);

    Vector va = math::mul(w, simd::load_float4(a.n_u.v));
    Vector vb = math::mul(u, simd::load_float4(b.n_u.v));
    va        = math::add(va, vb);
    Vector vc = math::mul(v, simd::load_float4(c.n_u.v));
    Vector v0 = math::add(va, vc);

    n = math::normalized3(v0);

    va        = math::mul(w, simd::load_float4(a.t_v.v));
    vb        = math::mul(u, simd::load_float4(b.t_v.v));
    va        = math::add(va, vb);
    vc        = math::mul(v, simd::load_float4(c.t_v.v));
    Vector v1 = math::add(va, vc);

    t = math::normalized3(v1);

    v0 = SU_MUX_HIGH(v0, v1);
    float4 r;
    simd::store_float4(r.v, v0);
    tc[0] = r[3];
    tc[1] = r[1];
}

static inline Vector interpolate_normal(FVector u, FVector v, const Shading_vertex_MTC& a,
                                        const Shading_vertex_MTC& b,
                                        const Shading_vertex_MTC& c) noexcept {
    Vector const w = math::sub(math::sub(simd::One, u), v);

    Vector va = math::mul(w, simd::load_float4(a.n_u.v));
    Vector vb = math::mul(u, simd::load_float4(b.n_u.v));
    va        = math::add(va, vb);
    Vector vc = math::mul(v, simd::load_float4(c.n_u.v));
    Vector v0 = math::add(va, vc);

    return math::normalized3(v0);
}

inline float xnorm_to_float(int16_t xnorm) noexcept {
    return float(xnorm) / 511.f;
}

inline int16_t float_to_xnorm(float x) noexcept {
    return static_cast<int16_t>(x * 511.f);
}

inline float4 snorm16_to_float(short4 v) noexcept {
    return float4(encoding::snorm16_to_float(v[0]), encoding::snorm16_to_float(v[1]),
                  encoding::snorm16_to_float(v[2]), xnorm_to_float(v[3]));
}

inline short4 float_to_snorm16(float3 const& v, float s) noexcept {
    return short4(encoding::float_to_snorm16(v[0]), encoding::float_to_snorm16(v[1]),
                  encoding::float_to_snorm16(v[2]), float_to_xnorm(s));
}

inline short4 float_to_snorm16(packed_float3 const& v, float s) noexcept {
    return short4(encoding::float_to_snorm16(v[0]), encoding::float_to_snorm16(v[1]),
                  encoding::float_to_snorm16(v[2]), float_to_xnorm(s));
}

inline Vertex_MTC::Vertex_MTC(packed_float3 const& p, packed_float3 const& n,
                              packed_float3 const& t, float2 uv) noexcept
    : p(p), n_u(n, uv[0]), t_v(t, uv[1]) {
    // Not too happy about handling degenerate tangents here (only one very special case even)
    if (0.f == t[0] && 0.f == t[1] && 0.f == t[2]) {
        t_v = float4(math::tangent(n_u.xyz()), uv[1]);
    }
}

inline bool intersect(const Vertex_MTC& a, const Vertex_MTC& b, const Vertex_MTC& c, ray& ray,
                      float2& uv) noexcept {
    float3 e1 = b.p - a.p;
    float3 e2 = c.p - a.p;

    float3 pvec = cross(ray.direction, e2);

    float det     = dot(e1, pvec);
    float inv_det = 1.f / det;

    float3 tvec = ray.origin - a.p;
    float  u    = dot(tvec, pvec) * inv_det;

    if (u < 0.f || u > 1.f) {
        return false;
    }

    float3 qvec = cross(tvec, e1);
    float  v    = dot(ray.direction, qvec) * inv_det;

    if (v < 0.f || u + v > 1.f) {
        return false;
    }

    float hit_t = dot(e2, qvec) * inv_det;

    if (hit_t > ray.min_t && hit_t < ray.max_t) {
        ray.max_t = hit_t;
        uv[0]     = u;
        uv[1]     = v;
        return true;
    }

    return false;
}

inline bool intersect_p(const Vertex_MTC& a, const Vertex_MTC& b, const Vertex_MTC& c,
                        ray const& ray) noexcept {
    float3 e1 = b.p - a.p;
    float3 e2 = c.p - a.p;

    float3 pvec = cross(ray.direction, e2);

    float det     = dot(e1, pvec);
    float inv_det = 1.f / det;

    float3 tvec = ray.origin - a.p;
    float  u    = dot(tvec, pvec) * inv_det;

    if (u < 0.f || u > 1.f) {
        return false;
    }

    float3 qvec = cross(tvec, e1);
    float  v    = dot(ray.direction, qvec) * inv_det;

    if (v < 0.f || u + v > 1.f) {
        return false;
    }

    float hit_t = dot(e2, qvec) * inv_det;

    if (hit_t > ray.min_t && hit_t < ray.max_t) {
        return true;
    }

    return false;
}

inline float2 interpolate_uv(const Vertex_MTC& a, const Vertex_MTC& b, const Vertex_MTC& c,
                             float2 uv) noexcept {
    float w = 1.f - uv[0] - uv[1];

    return float2(w * a.n_u[3] + uv[0] * b.n_u[3] + uv[1] * c.n_u[3],
                  w * a.t_v[3] + uv[0] * b.t_v[3] + uv[1] * c.t_v[3]);
}

inline void interpolate_p_uv(const Vertex_MTC& a, const Vertex_MTC& b, const Vertex_MTC& c,
                             float2 uv, float3& p, float2& tc) noexcept {
    float w = 1.f - uv[0] - uv[1];

    p = w * a.p + uv[0] * b.p + uv[1] * c.p;

    tc[0] = w * a.n_u[3] + uv[0] * b.n_u[3] + uv[1] * c.n_u[3];
    tc[1] = w * a.t_v[3] + uv[0] * b.t_v[3] + uv[1] * c.t_v[3];
}

inline void interpolate_data(const Vertex_MTC& a, const Vertex_MTC& b, const Vertex_MTC& c,
                             float2 uv, float3& n, float3& t, float2& tc) noexcept {
    float w = 1.f - uv[0] - uv[1];

    float4 n_u = w * a.n_u + uv[0] * b.n_u + uv[1] * c.n_u;
    float4 t_v = w * a.t_v + uv[0] * b.t_v + uv[1] * c.t_v;

    n = normalize(n_u.xyz());
    t = normalize(t_v.xyz());

    tc = float2(n_u[3], t_v[3]);
}

inline float area(const Vertex_MTC& a, const Vertex_MTC& b, const Vertex_MTC& c) noexcept {
    return 0.5f * length(cross(b.p - a.p, c.p - a.p));
}

inline float area(const Vertex_MTC& a, const Vertex_MTC& b, const Vertex_MTC& c,
                  float3 const& scale) noexcept {
    float3 sa = scale * a.p;
    float3 sb = scale * b.p;
    float3 sc = scale * c.p;
    return 0.5f * length(cross(sb - sa, sc - sa));
}

}  // namespace scene::shape::triangle

#endif
