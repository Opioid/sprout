#pragma once

#include <cstdint>

namespace math {

template<typename T> struct Vector3;
template<typename T> struct Vector4;

}

namespace color {

typedef math::Vector3<unsigned char> Color3c;
typedef math::Vector3<float> Color3;
typedef math::Vector4<unsigned char> Color4c;
typedef math::Vector4<float> Color4;

// convert sRGB linear color to sRGB gamma color
Color3 linear_to_sRGB(const Color3& c);

// convert sRGB gamma color to sRGB linear color
Color3 sRGB_to_linear(const Color3c& c);
Color3 sRGB_to_linear(const Color3& c);

Color4 sRGB_to_linear(const Color4c& c);

// convert linear color to gamma color
Color3 linear_to_gamma(const Color3& c, float gamma);

// convert gamma color to linear color
Color3 gamma_to_linear(const Color3& c, float gamma);

Color3 to_float(const Color3c& c);
Color4 to_float(const Color4c& c);

Color4c to_byte(const Color4& c);
uint32_t to_uint(const Color4& c);

float luminance(const Color3& c);

}
