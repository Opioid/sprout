#pragma once

#include "math/vector3.hpp"
#include "math/vector4.hpp"
#include "memory/const.hpp"

namespace spectrum {

SU_GLOBALCONST(float) CIE_constant = 683.002f;

// convert sRGB linear color to sRGB gamma color
float linear_to_sRGB(float c);

float3 linear_RGB_to_sRGB(float3_p c);
float4 linear_RGB_to_sRGB(float4_p c);

// convert sRGB gamma color to sRGB linear color
float sRGB_to_linear(float c);

float3 sRGB_to_linear_RGB(byte3 c);
float3 sRGB_to_linear_RGB(float3_p c);

float4 sRGB_to_linear_RGB(byte4 c);

// convert linear color to gamma color
float3 linear_to_gamma(float3_p c, float gamma);

// convert gamma color to linear color
float3 gamma_to_linear_RGB(float3_p c, float gamma);

float luminance(float3_p c);

float watt_to_lumen(float3_p c);

float radiance_to_nit(float3_p c);

}
