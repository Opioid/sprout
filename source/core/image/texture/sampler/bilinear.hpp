#ifndef SU_CORE_IMAGE_TEXTURE_SAMPLER_BILINEAR_HPP
#define SU_CORE_IMAGE_TEXTURE_SAMPLER_BILINEAR_HPP

#include "base/math/vector4.inl"

namespace image::texture::sampler {

static inline float bilinear(float c00, float c10, float c01, float c11, float s, float t) {
    float const _s = 1.f - s;
    float const _t = 1.f - t;

    return _t * (_s * c00 + s * c10) + t * (_s * c01 + s * c11);
}

static inline float bilinear(f_float4 c, float s, float t) {
    float const _s = 1.f - s;
    float const _t = 1.f - t;

    return _t * (_s * c[0] + s * c[1]) + t * (_s * c[2] + s * c[3]);
}

static inline float2 bilinear(float2 c00, float2 c10, float2 c01, float2 c11, float s, float t) {
    float const _s = 1.f - s;
    float const _t = 1.f - t;

    return _t * (_s * c00 + s * c10) + t * (_s * c01 + s * c11);
}

static inline float2 bilinear(float2 c[4], float s, float t) {
    float const _s = 1.f - s;
    float const _t = 1.f - t;

    return _t * (_s * c[0] + s * c[1]) + t * (_s * c[2] + s * c[3]);
}

static inline float3 bilinear(f_float3 c00, f_float3 c10, f_float3 c01, f_float3 c11, float s,
                              float t) {
    float const _s = 1.f - s;
    float const _t = 1.f - t;

    return _t * (_s * c00 + s * c10) + t * (_s * c01 + s * c11);
}

static inline float3 bilinear(float3 c[4], float s, float t) {
    float const _s = 1.f - s;
    float const _t = 1.f - t;

    return _t * (_s * c[0] + s * c[1]) + t * (_s * c[2] + s * c[3]);
}

static inline float4 bilinear(f_float4 c00, f_float4 c10, f_float4 c01, f_float4 c11, float s,
                              float t) {
    float const _s = 1.f - s;
    float const _t = 1.f - t;

    return _t * (_s * c00 + s * c10) + t * (_s * c01 + s * c11);
}

}  // namespace image::texture::sampler

#endif
