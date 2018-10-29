#include "texture_encoding.hpp"
#include "base/encoding/encoding.inl"
#include "base/spectrum/rgb.hpp"

namespace image::texture::encoding {

static float srgb_float[256];
static float snorm_float[256];
static float unorm_float[256];

void init() noexcept {
    for (uint32_t i = 0; i < 256; ++i) {
        srgb_float[i] = spectrum::sRGB_to_linear(static_cast<float>(i) * (1.f / 255.f));
    }

    for (uint32_t i = 0; i < 256; ++i) {
        snorm_float[i] = ::encoding::snorm_to_float(static_cast<uint8_t>(i));
    }

    for (uint32_t i = 0; i < 256; ++i) {
        unorm_float[i] = ::encoding::unorm_to_float(static_cast<uint8_t>(i));
    }
}

float cached_srgb_to_float(uint8_t byte) noexcept {
    return srgb_float[byte];
}

float2 cached_srgb_to_float2(byte3 const& byte) noexcept {
    return float2(srgb_float[byte[0]], srgb_float[byte[1]]);
}

float3 cached_srgb_to_float3(byte3 const& byte) noexcept {
    return float3(srgb_float[byte[0]], srgb_float[byte[1]], srgb_float[byte[2]]);
}

void cached_srgb_to_float(byte3 const bytes[4], float c[4]) noexcept {
    c[0] = srgb_float[bytes[0][0]];
    c[1] = srgb_float[bytes[1][0]];
    c[2] = srgb_float[bytes[2][0]];
    c[3] = srgb_float[bytes[3][0]];
}

void cached_srgb_to_float(byte3 const bytes[4], float2 c[4]) noexcept {
    c[0] = float2(srgb_float[bytes[0][0]], srgb_float[bytes[0][1]]);
    c[1] = float2(srgb_float[bytes[1][0]], srgb_float[bytes[1][1]]);
    c[2] = float2(srgb_float[bytes[2][0]], srgb_float[bytes[2][1]]);
    c[3] = float2(srgb_float[bytes[3][0]], srgb_float[bytes[3][1]]);
}

void cached_srgb_to_float(byte3 const bytes[4], float3 c[4]) noexcept {
    c[0] = float3(srgb_float[bytes[0][0]], srgb_float[bytes[0][1]], srgb_float[bytes[0][2]]);
    c[1] = float3(srgb_float[bytes[1][0]], srgb_float[bytes[1][1]], srgb_float[bytes[1][2]]);
    c[2] = float3(srgb_float[bytes[2][0]], srgb_float[bytes[2][1]], srgb_float[bytes[2][2]]);
    c[3] = float3(srgb_float[bytes[3][0]], srgb_float[bytes[3][1]], srgb_float[bytes[3][2]]);
}

float cached_snorm_to_float(uint8_t byte) noexcept {
    return snorm_float[byte];
}

void cached_snorm_to_float(byte3 const bytes[4], float3 c[4]) noexcept {
    c[0] = float3(snorm_float[bytes[0][0]], snorm_float[bytes[0][1]], snorm_float[bytes[0][2]]);
    c[1] = float3(snorm_float[bytes[1][0]], snorm_float[bytes[1][1]], snorm_float[bytes[1][2]]);
    c[2] = float3(snorm_float[bytes[2][0]], snorm_float[bytes[2][1]], snorm_float[bytes[2][2]]);
    c[3] = float3(snorm_float[bytes[3][0]], snorm_float[bytes[3][1]], snorm_float[bytes[3][2]]);
}

float cached_unorm_to_float(uint8_t byte) noexcept {
    return unorm_float[byte];
}

void cached_unorm_to_float(byte2 const bytes[4], float2 c[4]) noexcept {
    c[0] = float2(unorm_float[bytes[0][0]], unorm_float[bytes[0][1]]);
    c[1] = float2(unorm_float[bytes[1][0]], unorm_float[bytes[1][1]]);
    c[2] = float2(unorm_float[bytes[2][0]], unorm_float[bytes[2][1]]);
    c[3] = float2(unorm_float[bytes[3][0]], unorm_float[bytes[3][1]]);
}

}  // namespace image::texture::encoding
