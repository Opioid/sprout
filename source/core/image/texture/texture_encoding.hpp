#ifndef SU_CORE_IMAGE_TEXTURE_ENCODING_HPP
#define SU_CORE_IMAGE_TEXTURE_ENCODING_HPP

#include "base/math/vector.hpp"

namespace image::texture::encoding {

void init() noexcept;

float cached_srgb_to_float(uint8_t byte) noexcept;

float2 cached_srgb_to_float2(byte3 const& byte) noexcept;
float2 cached_srgb_to_float2(byte4 const& byte) noexcept;
float3 cached_srgb_to_float3(byte3 const& byte) noexcept;
float3 cached_srgb_to_float3(byte4 const& byte) noexcept;
float4 cached_srgb_to_float4(byte3 const& byte) noexcept;
float4 cached_srgb_to_float4(byte4 const& byte) noexcept;

void cached_srgb_to_float(byte3 const bytes[4], float c[4]) noexcept;
void cached_srgb_to_float(byte3 const bytes[4], float2 c[4]) noexcept;
void cached_srgb_to_float(byte3 const bytes[4], float3 c[4]) noexcept;

void cached_srgb_to_float(byte4 const bytes[4], float c[4]) noexcept;
void cached_srgb_to_float(byte4 const bytes[4], float2 c[4]) noexcept;
void cached_srgb_to_float(byte4 const bytes[4], float3 c[4]) noexcept;

float cached_snorm_to_float(uint8_t byte) noexcept;

void cached_snorm_to_float(byte3 const bytes[4], float3 c[4]) noexcept;

float cached_unorm_to_float(uint8_t byte) noexcept;

void cached_unorm_to_float(byte2 const bytes[4], float2 c[4]) noexcept;

}  // namespace image::texture::encoding

#endif
