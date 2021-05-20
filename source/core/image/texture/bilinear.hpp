#ifndef SU_CORE_IMAGE_TEXTURE_BILINEAR_HPP
#define SU_CORE_IMAGE_TEXTURE_BILINEAR_HPP

#include "base/math/matrix3x3.inl"
#include "base/math/vector4.inl"

namespace image::texture {

static inline float bilinear(float c00, float c10, float c01, float c11, float s, float t) {
    float const _s = 1.f - s;
    float const _t = 1.f - t;

    return _t * (_s * c00 + s * c10) + t * (_s * c01 + s * c11);
}

static inline float bilinear(float const c[4], float s, float t) {
    float const _s = 1.f - s;
    float const _t = 1.f - t;

    return _t * (_s * c[0] + s * c[1]) + t * (_s * c[2] + s * c[3]);
}

static inline float2 bilinear(float2 c00, float2 c10, float2 c01, float2 c11, float s, float t) {
    float const _s = 1.f - s;
    float const _t = 1.f - t;

    return _t * (_s * c00 + s * c10) + t * (_s * c01 + s * c11);
}

static inline float2 bilinear(float2 const c[4], float s, float t) {
    float const _s = 1.f - s;
    float const _t = 1.f - t;

    return _t * (_s * c[0] + s * c[1]) + t * (_s * c[2] + s * c[3]);
}

static inline float3 bilinear(float3_p c00, float3_p c10, float3_p c01, float3_p c11, float s,
                              float t) {
    float const _s = 1.f - s;
    float const _t = 1.f - t;

    return _t * (_s * c00 + s * c10) + t * (_s * c01 + s * c11);
}

static inline float4 bilinear(float4_p c00, float4_p c10, float4_p c01, float4_p c11, float s,
                              float t) {
    float const _s = 1.f - s;
    float const _t = 1.f - t;

    return _t * (_s * c00 + s * c10) + t * (_s * c01 + s * c11);
}

static inline float3x3 bilinear(float3x3 const& c00, float3x3 const& c10, float3x3 const& c01,
                                float3x3 const& c11, float s, float t) {
    float const _s = 1.f - s;
    float const _t = 1.f - t;

    return _t * (_s * c00 + s * c10) + t * (_s * c01 + s * c11);
}

static inline float3 bilinear(float3 const c[4], float s, float t) {
    float const _s = 1.f - s;
    float const _t = 1.f - t;

    return _t * (_s * c[0] + s * c[1]) + t * (_s * c[2] + s * c[3]);
}

static inline float3 bilinear_simd(float3 const c[4], float s, float t) {
    Simdf const ss(s);
    Simdf const st(t);

    Simdf const _s = Simdf(1.f) - ss;
    Simdf const _t = Simdf(1.f) - st;

    return float3(_t * (_s * Simdf(c[0]) + ss * Simdf(c[1])) +
                  st * (_s * Simdf(c[2]) + ss * Simdf(c[3])));
}

}  // namespace image::texture

#endif
