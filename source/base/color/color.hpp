#pragma once

#include "math/vector.hpp"
#include "memory/const.hpp"

namespace color {

SU_GLOBALCONST(float) CIE_constant = 683.002f;

// convert sRGB linear color to sRGB gamma color
math::float3 linear_to_sRGB(math::pfloat3 c);
math::float4 linear_to_sRGB(const math::float4& c);

// convert sRGB gamma color to sRGB linear color
float sRGB_to_linear(float c);

math::float3 sRGB_to_linear(math::byte3 c);
math::float3 sRGB_to_linear(math::pfloat3 c);

math::float4 sRGB_to_linear(math::byte4 c);

// convert linear color to gamma color
math::float3 linear_to_gamma(math::pfloat3 c, float gamma);

// convert gamma color to linear color
math::float3 gamma_to_linear(math::pfloat3 c, float gamma);

math::float3 to_float(math::byte3 c);
math::float4 to_float(math::byte4 c);

math::byte3 to_byte(math::pfloat3 c);
math::byte4 to_byte(const math::float4& c);
uint32_t to_uint(const math::float4& c);

float snorm_to_float(uint8_t byte);
float unorm_to_float(uint8_t byte);

uint8_t float_to_snorm(float x);

float luminance(math::pfloat3 c);

float watt_to_lumen(math::pfloat3 c);

float radiance_to_nit(math::pfloat3 c);

}
