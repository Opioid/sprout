#ifndef SU_CORE_SCENE_SHAPE_TRIANGLE_PRIMITIVE_MT_INL
#define SU_CORE_SCENE_SHAPE_TRIANGLE_PRIMITIVE_MT_INL

#include "base/encoding/encoding.inl"
#include "base/math/ray.hpp"
#include "triangle_primitive_mt.hpp"

namespace scene::shape::triangle {

inline Triangle_MT::Triangle_MT(const shape::Vertex& a, const shape::Vertex& b,
                                const shape::Vertex& c, float bitangent_sign,
                                uint32_t material_index)
    : a(a), b(b), c(c), bitangent_sign(bitangent_sign), material_index(material_index) {}

inline Triangle_MT::Vertex::Vertex(const shape::Vertex& v) : p(v.p), n(v.n), t(v.t), uv(v.uv) {}

inline bool Triangle_MT::intersect(ray& ray, float2& uv) const {
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

    if (hit_t > ray.min_t() && hit_t < ray.max_t()) {
        ray.max_t() = hit_t;
        uv[0]       = u;
        uv[1]       = v;
        return true;
    }

    return false;
}

inline bool Triangle_MT::intersect_p(ray const& ray) const {
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

    if (hit_t > ray.min_t() && hit_t < ray.max_t()) {
        return true;
    }

    return false;
}

inline void Triangle_MT::interpolate(float2 uv, float3& p, float3& n, float2& tc) const {
    float w = 1.f - uv[0] - uv[1];

    p  = w * a.p + uv[0] * b.p + uv[1] * c.p;
    n  = normalize(w * a.n + uv[0] * b.n + uv[1] * c.n);
    tc = w * a.uv + uv[0] * b.uv + uv[1] * c.uv;
}

inline void Triangle_MT::interpolate(float2 uv, float3& p, float2& tc) const {
    float w = 1.f - uv[0] - uv[1];

    p  = w * a.p + uv[0] * b.p + uv[1] * c.p;
    tc = w * a.uv + uv[0] * b.uv + uv[1] * c.uv;
}

inline void Triangle_MT::interpolate(float2 uv, float3& p) const {
    float w = 1.f - uv[0] - uv[1];

    p = w * a.p + uv[0] * b.p + uv[1] * c.p;
}

inline void Triangle_MT::interpolate_data(float2 uv, float3& n, float3& t, float2& tc) const {
    float w = 1.f - uv[0] - uv[1];

    n  = normalize(w * a.n + uv[0] * b.n + uv[1] * c.n);
    t  = normalize(w * a.t + uv[0] * b.t + uv[1] * c.t);
    tc = w * a.uv + uv[0] * b.uv + uv[1] * c.uv;
}

inline float2 Triangle_MT::interpolate_uv(float2 uv) const {
    float w = 1.f - uv[0] - uv[1];

    return w * a.uv + uv[0] * b.uv + uv[1] * c.uv;
}

inline float3 Triangle_MT::normal() const {
    float3 e1 = b.p - a.p;
    float3 e2 = c.p - a.p;

    return normalize(cross(e1, e2));
}

inline float Triangle_MT::area() const {
    return 0.5f * length(cross(b.p - a.p, c.p - a.p));
}

inline float Triangle_MT::area(float3 const& scale) const {
    float3 sa = scale * a.p;
    float3 sb = scale * b.p;
    float3 sc = scale * c.p;
    return 0.5f * length(cross(sb - sa, sc - sa));
}

static inline bool intersect(float3 const& a, float3 const& b, float3 const& c, ray& ray,
                             float2& uv) {
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

    //	if (hit_t > ray.min_t() && hit_t < ray.max_t()) {
    //		ray.max_t() = hit_t;
    //		uv[0] = u;
    //		uv[1] = v;
    //		return true;
    //	}

    //	return false;

    uint8_t ca = uint8_t(u > 0.f);
    uint8_t cb = uint8_t(u < 1.f);
    uint8_t cc = uint8_t(v > 0.f);
    uint8_t cd = uint8_t(u + v < 1.f);
    uint8_t ce = uint8_t(hit_t > ray.min_t());
    uint8_t cf = uint8_t(hit_t < ray.max_t());

    if (0 != (ca & cb & cc & cd & ce & cf)) {
        ray.max_t() = hit_t;
        uv[0]       = u;
        uv[1]       = v;
        return true;
    }

    return false;
}

static inline bool intersect(Simd3f const& origin, Simd3f const& direction, scalar const& min_t,
                             scalar& max_t, float const* a, float const* b, float const* c,
                             scalar& u_out, scalar& v_out) {
    Simd3f ap(a);
    Simd3f bp(b);
    Simd3f cp(c);

    Simd3f e1   = bp - ap;
    Simd3f e2   = cp - ap;
    Simd3f tvec = origin - ap;

    Simd3f pvec = cross(direction, e2);
    Simd3f qvec = cross(tvec, e1);

    scalar e1_d_pv = dot_scalar(e1, pvec);
    scalar tv_d_pv = dot_scalar(tvec, pvec);
    scalar di_d_qv = dot_scalar(direction, qvec);
    scalar e2_d_qv = dot_scalar(e2, qvec);

    scalar inv_det = reciprocal(e1_d_pv);

    scalar u     = tv_d_pv * inv_det;
    scalar v     = di_d_qv * inv_det;
    scalar hit_t = e2_d_qv * inv_det;

    scalar uv = u + v;

    if (0 != (_mm_ucomige_ss(u.v, simd::Zero) & _mm_ucomige_ss(simd::One, u.v) &
              _mm_ucomige_ss(v.v, simd::Zero) & _mm_ucomige_ss(simd::One, uv.v) &
              _mm_ucomige_ss(hit_t.v, min_t.v) & _mm_ucomige_ss(max_t.v, hit_t.v))) {
        max_t = hit_t;
        u_out = u;
        v_out = v;
        return true;
    }

    return false;
}

static inline bool intersect(Simd3f const& origin, Simd3f const& direction, scalar const& min_t,
                             scalar& max_t, float const* a, float const* b, float const* c) {
    Simd3f ap(a);
    Simd3f bp(b);
    Simd3f cp(c);

    Simd3f e1   = bp - ap;
    Simd3f e2   = cp - ap;
    Simd3f tvec = origin - ap;

    Simd3f pvec = cross(direction, e2);
    Simd3f qvec = cross(tvec, e1);

    scalar e1_d_pv = dot_scalar(e1, pvec);
    scalar tv_d_pv = dot_scalar(tvec, pvec);
    scalar di_d_qv = dot_scalar(direction, qvec);
    scalar e2_d_qv = dot_scalar(e2, qvec);

    scalar inv_det = reciprocal(e1_d_pv);

    scalar u     = tv_d_pv * inv_det;
    scalar v     = di_d_qv * inv_det;
    scalar hit_t = e2_d_qv * inv_det;

    scalar uv = u + v;

    if (0 != (_mm_ucomige_ss(u.v, simd::Zero) & _mm_ucomige_ss(simd::One, u.v) &
              _mm_ucomige_ss(v.v, simd::Zero) & _mm_ucomige_ss(simd::One, uv.v) &
              _mm_ucomige_ss(hit_t.v, min_t.v) & _mm_ucomige_ss(max_t.v, hit_t.v))) {
        max_t = hit_t;
        return true;
    }

    return false;
}

static inline bool intersect_p(float3 const& a, float3 const& b, float3 const& c, ray const& ray) {
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

            if (hit_t > ray.min_t() && hit_t < ray.max_t()) {
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
    uint8_t ce = uint8_t(hit_t > ray.min_t());
    uint8_t cf = uint8_t(hit_t < ray.max_t());

    return 0 != (ca & cb & cc & cd & ce & cf);
}

static inline bool intersect_p(Simd3f const& origin, Simd3f const& direction, scalar const& min_t,
                               scalar const& max_t, float const* a, float const* b,
                               float const* c) {
    // Implementation C

    Simd3f ap(a);
    Simd3f bp(b);
    Simd3f cp(c);

    Simd3f e1   = bp - ap;
    Simd3f e2   = cp - ap;
    Simd3f tvec = origin - ap;

    Simd3f pvec = cross(direction, e2);
    Simd3f qvec = cross(tvec, e1);

    scalar e1_d_pv = dot_scalar(e1, pvec);
    scalar tv_d_pv = dot_scalar(tvec, pvec);
    scalar di_d_qv = dot_scalar(direction, qvec);
    scalar e2_d_qv = dot_scalar(e2, qvec);

    scalar inv_det = reciprocal(e1_d_pv);

    scalar u     = tv_d_pv * inv_det;
    scalar v     = di_d_qv * inv_det;
    scalar hit_t = e2_d_qv * inv_det;

    scalar uv = u + v;

    return 0 != (_mm_ucomige_ss(u.v, simd::Zero) & _mm_ucomige_ss(simd::One, u.v) &
                 _mm_ucomige_ss(v.v, simd::Zero) & _mm_ucomige_ss(simd::One, uv.v) &
                 _mm_ucomige_ss(hit_t.v, min_t.v) & _mm_ucomige_ss(max_t.v, hit_t.v));
}

static inline Simd3f interpolate_p(Simd3f const& a, Simd3f const& b, Simd3f const& c,
                                   Simd3f const& u, Simd3f const& v) {
    Simd3f const w = simd::One - u - v;

    return w * a + u * b + v * c;
}

static inline void interpolate_p(float3 const& a, float3 const& b, float3 const& c, float2 uv,
                                 float3& p) {
    float const w = 1.f - uv[0] - uv[1];

    p = w * a + uv[0] * b + uv[1] * c;
}

static inline float area(float3 const& a, float3 const& b, float3 const& c) {
    return 0.5f * length(cross(b - a, c - a));
}

static inline float area(float3 const& a, float3 const& b, float3 const& c, float3 const& scale) {
    float3 const sa = scale * a;
    float3 const sb = scale * b;
    float3 const sc = scale * c;
    return 0.5f * length(cross(sb - sa, sc - sa));
}

static inline float2 interpolate_uv(const Shading_vertex_MT& a, const Shading_vertex_MT& b,
                                    const Shading_vertex_MT& c, float2 uv) {
    float const w = 1.f - uv[0] - uv[1];

    return w * a.uv + uv[0] * b.uv + uv[1] * c.uv;
}

static inline void interpolate_data(const Shading_vertex_MT& a, const Shading_vertex_MT& b,
                                    const Shading_vertex_MT& c, float2 uv, float3& n, float3& t,
                                    float2& tc) {
    float const w = 1.f - uv[0] - uv[1];

    n  = normalize(w * a.n + uv[0] * b.n + uv[1] * c.n);
    t  = normalize(w * a.t + uv[0] * b.t + uv[1] * c.t);
    tc = w * a.uv + uv[0] * b.uv + uv[1] * c.uv;
}

inline Shading_vertex_MTC::Shading_vertex_MTC(float3 const& n, float3 const& t, float2 uv)
    : n_u(n, uv[0]), t_v(t, uv[1]) {
    // Not too happy about handling degenerate tangents here (only one very special case even)
    if (0.f == t[0] && 0.f == t[1] && 0.f == t[2]) {
        t_v = float4(math::tangent(n_u.xyz()), uv[1]);
    }
}

static inline float2 interpolate_uv(const Shading_vertex_MTC& a, const Shading_vertex_MTC& b,
                                    const Shading_vertex_MTC& c, float2 uv) {
    float const w = 1.f - uv[0] - uv[1];

    return float2(w * a.n_u[3] + uv[0] * b.n_u[3] + uv[1] * c.n_u[3],
                  w * a.t_v[3] + uv[0] * b.t_v[3] + uv[1] * c.t_v[3]);
}

static inline float2 interpolate_uv(Simd3f const& u, Simd3f const& v, const Shading_vertex_MTC& a,
                                    const Shading_vertex_MTC& b, const Shading_vertex_MTC& c) {
    Simd3f const w = simd::One - u - v;

    Simd3f va(a.n_u[3], a.t_v[3], 0.f);
    Simd3f vb(b.n_u[3], b.t_v[3], 0.f);

    va = w * va;
    vb = u * vb;
    va = va + vb;

    Simd3f vc(c.n_u[3], c.t_v[3], 0.f);

    vc = v * vc;

    Simd3f const uv = va + vc;

    return float3(uv).xy();
}

static inline void interpolate_data(const Shading_vertex_MTC& a, const Shading_vertex_MTC& b,
                                    const Shading_vertex_MTC& c, float2 uv, float3& n, float3& t,
                                    float2& tc) {
    float const w = 1.f - uv[0] - uv[1];

    float4 const n_u = w * a.n_u + uv[0] * b.n_u + uv[1] * c.n_u;
    float4 const t_v = w * a.t_v + uv[0] * b.t_v + uv[1] * c.t_v;

    n = normalize(n_u.xyz());
    t = normalize(t_v.xyz());

    tc = float2(n_u[3], t_v[3]);
}

static inline void interpolate_data(Simd3f const& u, Simd3f const& v, const Shading_vertex_MTC& a,
                                    const Shading_vertex_MTC& b, const Shading_vertex_MTC& c,
                                    Simd3f& n, Simd3f& t, float2& tc) {
    Simd3f const w = simd::One - u - v;

    Simd3f va = w * Simd3f(a.n_u.v);
    Simd3f vb = u * Simd3f(b.n_u.v);

    va = va + vb;

    Simd3f vc = v * Simd3f(c.n_u.v);
    Simd3f v0 = va + vc;

    n = normalize(v0);

    va = w * Simd3f(a.t_v.v);
    vb = u * Simd3f(b.t_v.v);
    va = va + vb;
    vc = v * Simd3f(c.t_v.v);

    Simd3f const v1 = va + vc;

    t = normalize(v1);

    v0 = SU_MUX_HIGH(v0.v, v1.v);
    float4 r;
    simd::store_float4(r.v, v0.v);
    tc[0] = r[3];
    tc[1] = r[1];
    //    tc[0] = v0.w();
    //    tc[1] = v1.w();
}

static inline Simd3f interpolate_normal(Simd3f const& u, Simd3f const& v,
                                        const Shading_vertex_MTC& a, const Shading_vertex_MTC& b,
                                        const Shading_vertex_MTC& c) {
    Simd3f const w = simd::One - u - v;

    Simd3f va = w * Simd3f(a.n_u.v);
    Simd3f vb = u * Simd3f(b.n_u.v);

    va = va + vb;

    Simd3f vc = v * Simd3f(c.n_u.v);
    Simd3f v0 = va + vc;

    return normalize(v0);
}

inline Vertex_MTC::Vertex_MTC(packed_float3 const& p, packed_float3 const& n,
                              packed_float3 const& t, float2 uv)
    : p(p), n_u(n, uv[0]), t_v(t, uv[1]) {
    // Not too happy about handling degenerate tangents here (only one very special case even)
    if (0.f == t[0] && 0.f == t[1] && 0.f == t[2]) {
        t_v = float4(math::tangent(n_u.xyz()), uv[1]);
    }
}

inline bool intersect(const Vertex_MTC& a, const Vertex_MTC& b, const Vertex_MTC& c, ray& ray,
                      float2& uv) {
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

    if (hit_t > ray.min_t() && hit_t < ray.max_t()) {
        ray.max_t() = hit_t;
        uv[0]       = u;
        uv[1]       = v;
        return true;
    }

    return false;
}

inline bool intersect_p(const Vertex_MTC& a, const Vertex_MTC& b, const Vertex_MTC& c,
                        ray const& ray) {
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

    if (hit_t > ray.min_t() && hit_t < ray.max_t()) {
        return true;
    }

    return false;
}

inline float2 interpolate_uv(const Vertex_MTC& a, const Vertex_MTC& b, const Vertex_MTC& c,
                             float2 uv) {
    float w = 1.f - uv[0] - uv[1];

    return float2(w * a.n_u[3] + uv[0] * b.n_u[3] + uv[1] * c.n_u[3],
                  w * a.t_v[3] + uv[0] * b.t_v[3] + uv[1] * c.t_v[3]);
}

inline void interpolate_p_uv(const Vertex_MTC& a, const Vertex_MTC& b, const Vertex_MTC& c,
                             float2 uv, float3& p, float2& tc) {
    float w = 1.f - uv[0] - uv[1];

    p = w * a.p + uv[0] * b.p + uv[1] * c.p;

    tc[0] = w * a.n_u[3] + uv[0] * b.n_u[3] + uv[1] * c.n_u[3];
    tc[1] = w * a.t_v[3] + uv[0] * b.t_v[3] + uv[1] * c.t_v[3];
}

inline void interpolate_data(const Vertex_MTC& a, const Vertex_MTC& b, const Vertex_MTC& c,
                             float2 uv, float3& n, float3& t, float2& tc) {
    float w = 1.f - uv[0] - uv[1];

    float4 n_u = w * a.n_u + uv[0] * b.n_u + uv[1] * c.n_u;
    float4 t_v = w * a.t_v + uv[0] * b.t_v + uv[1] * c.t_v;

    n = normalize(n_u.xyz());
    t = normalize(t_v.xyz());

    tc = float2(n_u[3], t_v[3]);
}

inline float area(const Vertex_MTC& a, const Vertex_MTC& b, const Vertex_MTC& c) {
    return 0.5f * length(cross(b.p - a.p, c.p - a.p));
}

inline float area(const Vertex_MTC& a, const Vertex_MTC& b, const Vertex_MTC& c,
                  float3 const& scale) {
    float3 sa = scale * a.p;
    float3 sb = scale * b.p;
    float3 sc = scale * c.p;
    return 0.5f * length(cross(sb - sa, sc - sa));
}

}  // namespace scene::shape::triangle

#endif
