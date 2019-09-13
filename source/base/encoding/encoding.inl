#ifndef SU_BASE_ENCODING_ENCODING_INL
#define SU_BASE_ENCODING_ENCODING_INL

#include "math/vector3.inl"
#include "math/vector4.inl"

namespace encoding {

static inline float constexpr unorm_to_float(uint8_t byte) {
    return float(byte) * (1.f / 255.f);
}

static inline float3 constexpr unorm_to_float(byte3 c) {
    return float3(unorm_to_float(c[0]), unorm_to_float(c[1]), unorm_to_float(c[2]));
}

static inline float4 constexpr unorm_to_float(byte4 c) {
    return float4(unorm_to_float(c[0]), unorm_to_float(c[1]), unorm_to_float(c[2]),
                  unorm_to_float(c[3]));
}

static inline uint8_t constexpr float_to_unorm(float x) {
    return uint8_t(x * 255.f + 0.5f);
}

static inline uint8_t constexpr float_to_unorm(float x, float dither) {
    return uint8_t(x * 255.f + dither);
}

static inline byte3 constexpr float_to_unorm(float3 const& c) {
    return byte3(float_to_unorm(c[0]), float_to_unorm(c[1]), float_to_unorm(c[2]));
}

static inline byte3 constexpr float_to_unorm(float3 const& c, float dither) {
    return byte3(float_to_unorm(c[0], dither), float_to_unorm(c[1], dither),
                 float_to_unorm(c[2], dither));
}

static inline byte3 constexpr float_to_unorm(float3 const& c, float3 const& dither) {
    return byte3(float_to_unorm(c[0], dither[0]), float_to_unorm(c[1], dither[1]),
                 float_to_unorm(c[2], dither[2]));
}

static inline constexpr byte4 float_to_unorm(float4 const& c) {
    return byte4(float_to_unorm(c[0]), float_to_unorm(c[1]), float_to_unorm(c[2]),
                 float_to_unorm(c[3]));
}

static inline float constexpr snorm_to_float(uint8_t byte) {
    return float(byte) * (1.f / 128.f) - 1.f;
}

static inline uint8_t constexpr float_to_snorm(float x) {
    //	return uint8_t((x + 1.f) * (0.5f * 255.f));
    return uint8_t((x + 1.f) * (x > 0.f ? 127.5f : 128.f));
}

static inline byte3 constexpr float_to_snorm(float3 const& c) {
    return byte3(float_to_snorm(c[0]), float_to_snorm(c[1]), float_to_snorm(c[2]));
}

static inline float constexpr snorm16_to_float(int16_t snorm) {
    return float(snorm) * (1.f / 32767.f);
}

static inline constexpr int16_t float_to_snorm16(float x) {
    return static_cast<int16_t>(x * 32767.f);
}

}  // namespace encoding

#endif
