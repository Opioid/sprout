#ifndef SU_CORE_IMAGE_TEXTURE_ENCODING_HPP
#define SU_CORE_IMAGE_TEXTURE_ENCODING_HPP

#include "base/math/vector.hpp"

namespace image::texture::encoding {

void init();

float cached_srgb_to_float(uint8_t byte);

float2 cached_srgb_to_float2(byte3 const& byte);
float2 cached_srgb_to_float2(byte4 const& byte);
float3 cached_srgb_to_float3(byte3 const& byte);
float3 cached_srgb_to_float3(byte4 const& byte);
float4 cached_srgb_to_float4(byte4 const& byte);

void cached_srgb_to_float(byte3 const bytes[4], float c[4]);
void cached_srgb_to_float(byte3 const bytes[4], float2 c[4]);
void cached_srgb_to_float(byte3 const bytes[4], float3 c[4]);

void cached_srgb_to_float(byte4 const bytes[4], float c[4]);
void cached_srgb_to_float(byte4 const bytes[4], float2 c[4]);
void cached_srgb_to_float(byte4 const bytes[4], float3 c[4]);

float cached_snorm_to_float(uint8_t byte);

void cached_snorm_to_float(byte3 const bytes[4], float3 c[4]);

float cached_unorm_to_float(uint8_t byte);

void cached_unorm_to_float(byte2 const bytes[4], float2 c[4]);

}  // namespace image::texture::encoding

#endif
