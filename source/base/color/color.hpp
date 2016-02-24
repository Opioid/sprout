#pragma once

#include "math/vector.hpp"

namespace color {

typedef math::Vector2<uint8_t> Color2c;
typedef math::Vector3<uint8_t> Color3c;
//typedef math::Vector3<float>   Color3;
//typedef math::Vector3f_a	   Color3_a;
typedef math::vec3 Color3;
typedef math::Vector4<uint8_t> Color4c;
//typedef math::Vector4<float>   Color4;
typedef math::float4		   Color4;

// convert sRGB linear color to sRGB gamma color
Color3 linear_to_sRGB(math::pvec3 c);
Color4 linear_to_sRGB(const Color4& c);

// convert sRGB gamma color to sRGB linear color
float sRGB_to_linear(float c);

Color3 sRGB_to_linear(Color3c c);
Color3 sRGB_to_linear(math::pvec3 c);

Color4 sRGB_to_linear(Color4c c);

// convert linear color to gamma color
Color3 linear_to_gamma(math::pvec3 c, float gamma);

// convert gamma color to linear color
Color3 gamma_to_linear(math::pvec3 c, float gamma);

Color3 to_float(Color3c c);
Color4 to_float(Color4c c);

Color4c to_byte(const Color4& c);
uint32_t to_uint(const Color4& c);

float snorm_to_float(uint8_t byte);
float unorm_to_float(uint8_t byte);

uint8_t float_to_snorm(float x);

float luminance(math::pvec3 c);

}
