#ifndef SU_CORE_SCENE_SHAPE_TRIANGLE_PRIMITIVE_MT_INL
#define SU_CORE_SCENE_SHAPE_TRIANGLE_PRIMITIVE_MT_INL

#include "base/encoding/encoding.inl"
#include "base/math/ray.hpp"
#include "triangle_primitive_mt.hpp"

namespace scene::shape::triangle {

static inline bool intersect(float3_p a, float3_p b, float3_p c, ray& ray, float2& uv) {
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

static inline bool intersect(Simdf_p origin, Simdf_p direction, scalar_p min_t, scalar& max_t,
                             float const* a, float const* b, float const* c, scalar& u_out,
                             scalar& v_out) {
    Simdf ap(a);
    Simdf bp(b);
    Simdf cp(c);

    Simdf e1   = bp - ap;
    Simdf e2   = cp - ap;
    Simdf tvec = origin - ap;

    Simdf pvec = cross3(direction, e2);
    Simdf qvec = cross3(tvec, e1);

    scalar e1_d_pv = dot3_scalar(e1, pvec);
    scalar tv_d_pv = dot3_scalar(tvec, pvec);
    scalar di_d_qv = dot3_scalar(direction, qvec);
    scalar e2_d_qv = dot3_scalar(e2, qvec);

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

static inline bool intersect(Simdf_p origin, Simdf_p direction, scalar_p min_t, scalar& max_t,
                             float const* a, float const* b, float const* c) {
    Simdf ap(a);
    Simdf bp(b);
    Simdf cp(c);

    Simdf e1   = bp - ap;
    Simdf e2   = cp - ap;
    Simdf tvec = origin - ap;

    Simdf pvec = cross3(direction, e2);
    Simdf qvec = cross3(tvec, e1);

    scalar e1_d_pv = dot3_scalar(e1, pvec);
    scalar tv_d_pv = dot3_scalar(tvec, pvec);
    scalar di_d_qv = dot3_scalar(direction, qvec);
    scalar e2_d_qv = dot3_scalar(e2, qvec);

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

static inline bool intersect_p(float3_p a, float3_p b, float3_p c, ray const& ray) {
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

static inline bool intersect_p(Simdf_p origin, Simdf_p direction, scalar_p min_t, scalar_p max_t,
                               float const* a, float const* b, float const* c) {
    // Implementation C

    Simdf ap(a);
    Simdf bp(b);
    Simdf cp(c);

    Simdf e1   = bp - ap;
    Simdf e2   = cp - ap;
    Simdf tvec = origin - ap;

    Simdf pvec = cross3(direction, e2);
    Simdf qvec = cross3(tvec, e1);

    scalar e1_d_pv = dot3_scalar(e1, pvec);
    scalar tv_d_pv = dot3_scalar(tvec, pvec);
    scalar di_d_qv = dot3_scalar(direction, qvec);
    scalar e2_d_qv = dot3_scalar(e2, qvec);

    scalar inv_det = reciprocal(e1_d_pv);

    scalar u     = tv_d_pv * inv_det;
    scalar v     = di_d_qv * inv_det;
    scalar hit_t = e2_d_qv * inv_det;

    scalar uv = u + v;

    return 0 != (_mm_ucomige_ss(u.v, simd::Zero) & _mm_ucomige_ss(simd::One, u.v) &
                 _mm_ucomige_ss(v.v, simd::Zero) & _mm_ucomige_ss(simd::One, uv.v) &
                 _mm_ucomige_ss(hit_t.v, min_t.v) & _mm_ucomige_ss(max_t.v, hit_t.v));
}

static inline Simdf interpolate_p(Simdf_p a, Simdf_p b, Simdf_p c, Simdf_p u, Simdf_p v) {
    Simdf const w = simd::One - u - v;

    return w * a + u * b + v * c;
}

static inline void interpolate_p(float3_p a, float3_p b, float3_p c, float2 uv, float3& p) {
    float const w = 1.f - uv[0] - uv[1];

    p = w * a + uv[0] * b + uv[1] * c;
}

static inline float area(float3_p a, float3_p b, float3_p c) {
    return 0.5f * length(cross(b - a, c - a));
}

inline Shading_vertex_MTC::Shading_vertex_MTC() = default;

inline Shading_vertex_MTC::Shading_vertex_MTC(float3_p n, float3_p t, float2 uv)
    : n_u(n, uv[0]), t_v(t, uv[1]) {}

static inline float2 interpolate_uv(const Shading_vertex_MTC& a, const Shading_vertex_MTC& b,
                                    const Shading_vertex_MTC& c, float2 uv) {
    float const w = 1.f - uv[0] - uv[1];

    return float2(w * a.n_u[3] + uv[0] * b.n_u[3] + uv[1] * c.n_u[3],
                  w * a.t_v[3] + uv[0] * b.t_v[3] + uv[1] * c.t_v[3]);
}

static inline float2 interpolate_uv(Simdf_p u, Simdf_p v, const Shading_vertex_MTC& a,
                                    const Shading_vertex_MTC& b, const Shading_vertex_MTC& c) {
    Simdf const w = simd::One - u - v;

    Simdf va(a.n_u[3], a.t_v[3], 0.f);
    Simdf vb(b.n_u[3], b.t_v[3], 0.f);

    va = w * va;
    vb = u * vb;
    va = va + vb;

    Simdf vc(c.n_u[3], c.t_v[3], 0.f);

    vc = v * vc;

    Simdf const uv = va + vc;

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

static inline void interpolate_data(Simdf_p u, Simdf_p v, const Shading_vertex_MTC& a,
                                    const Shading_vertex_MTC& b, const Shading_vertex_MTC& c,
                                    Simdf& n, Simdf& t, float2& tc) {
    Simdf const w = simd::One - u - v;

    Simdf va = w * Simdf(a.n_u.v);
    Simdf vb = u * Simdf(b.n_u.v);

    va = va + vb;

    Simdf vc = v * Simdf(c.n_u.v);
    Simdf v0 = va + vc;

    n = normalize3(v0);

    va = w * Simdf(a.t_v.v);
    vb = u * Simdf(b.t_v.v);
    va = va + vb;
    vc = v * Simdf(c.t_v.v);

    Simdf const v1 = va + vc;

    t = normalize3(v1);

    v0 = SU_MUX_HIGH(v0.v, v1.v);
    float4 r;
    simd::store_float4(r.v, v0.v);
    tc[0] = r[3];
    tc[1] = r[1];
    //    tc[0] = v0.w();
    //    tc[1] = v1.w();
}

static inline Simdf interpolate_normal(Simdf_p u, Simdf_p v, const Shading_vertex_MTC& a,
                                       const Shading_vertex_MTC& b, const Shading_vertex_MTC& c) {
    Simdf const w = simd::One - u - v;

    Simdf va = w * Simdf(a.n_u.v);
    Simdf vb = u * Simdf(b.n_u.v);

    va = va + vb;

    Simdf vc = v * Simdf(c.n_u.v);
    Simdf v0 = va + vc;

    return normalize3(v0);
}

}  // namespace scene::shape::triangle

#endif
