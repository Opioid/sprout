#pragma once

#include "math/vector.hpp"
#include "memory/const.hpp"

namespace spectrum {

SU_GLOBALCONST(float) CIE_constant = 683.002f;

// convert sRGB linear color to sRGB gamma color
float3 linear_RGB_to_sRGB(float3_p c);
float4 linear_RGB_to_sRGB(const float4& c);

// convert sRGB gamma color to sRGB linear color
float sRGB_to_linear(float c);

float3 sRGB_to_linear_RGB(math::byte3 c);
float3 sRGB_to_linear_RGB(float3_p c);

float4 sRGB_to_linear_RGB(math::byte4 c);

// convert linear color to gamma color
float3 linear_to_gamma(float3_p c, float gamma);

// convert gamma color to linear color
float3 gamma_to_linear_RGB(float3_p c, float gamma);

float3 unorm_to_float(math::byte3 c);
float4 unorm_to_float(math::byte4 c);

byte3 float_to_unorm(float3_p c);
math::byte4 float_to_unorm(const float4& c);

uint32_t to_uint(const float4& c);

float unorm_to_float(uint8_t byte);
float snorm_to_float(uint8_t byte);

uint8_t float_to_unorm(float x);
uint8_t float_to_snorm(float x);

byte3 float_to_snorm(float3_p c);

float luminance(float3_p c);

float watt_to_lumen(float3_p c);

float radiance_to_nit(float3_p c);

}
