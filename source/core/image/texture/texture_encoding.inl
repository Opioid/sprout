#ifndef SU_CORE_IMAGE_TEXTURE_ENCODING_INL
#define SU_CORE_IMAGE_TEXTURE_ENCODING_INL

#include "base/math/vector4.inl"

namespace image::texture::encoding {

#include "texture_encoding_tables.inl"

static inline float cached_srgb_to_float(uint8_t byte) {
    return SRGB_FLOAT[byte];
}

static inline float2 cached_srgb_to_float(byte2 byte) {
    return float2(SRGB_FLOAT[byte[0]], SRGB_FLOAT[byte[1]]);
}

static inline float3 cached_srgb_to_float(byte3 const& byte) {
    return float3(SRGB_FLOAT[byte[0]], SRGB_FLOAT[byte[1]], SRGB_FLOAT[byte[2]]);
}

static inline float4 cached_srgb_to_float(byte4 const& byte) {
    return float4(SRGB_FLOAT[byte[0]], SRGB_FLOAT[byte[1]], SRGB_FLOAT[byte[2]],
                  UNORM_FLOAT[byte[3]]);
}

static inline float cached_snorm_to_float(uint8_t byte) {
    return SNORM_FLOAT[byte];
}

static inline float3 cached_snorm_to_float(byte3 const& byte) {
    return float3(SNORM_FLOAT[byte[0]], SNORM_FLOAT[byte[1]], SNORM_FLOAT[byte[2]]);
}

static inline float cached_unorm_to_float(uint8_t byte) {
    return UNORM_FLOAT[byte];
}

static inline float2 cached_unorm_to_float(byte2 byte) {
    return float2(UNORM_FLOAT[byte[0]], UNORM_FLOAT[byte[1]]);
}

}  // namespace image::texture::encoding

#endif
