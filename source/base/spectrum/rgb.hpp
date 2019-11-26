#ifndef SU_BASE_SPECTRUM_RGB_HPP
#define SU_BASE_SPECTRUM_RGB_HPP

#include "math/vector4.inl"

namespace spectrum {

// convert sRGB linear value to sRGB gamma value
static inline float linear_to_gamma_sRGB(float c) {
    if (c <= 0.f) {
        return 0.f;
    } else if (c < 0.0031308f) {
        return 12.92f * c;
    } else if (c < 1.f) {
        return 1.055f * std::pow(c, 0.41666f) - 0.055f;
    }

    return 1.f;
}

// convert sRGB linear color to sRGB gamma color
static inline float3 linear_to_gamma_sRGB(float3 const& c) {
    return float3(linear_to_gamma_sRGB(c[0]), linear_to_gamma_sRGB(c[1]),
                  linear_to_gamma_sRGB(c[2]));
}

// convert sRGB linear color to sRGB gamma color
static inline float4 linear_to_gamma_sRGB(float4 const& c) {
    return float4(linear_to_gamma_sRGB(c[0]), linear_to_gamma_sRGB(c[1]),
                  linear_to_gamma_sRGB(c[2]), c[3]);
}

// convert sRGB gamma value to sRGB linear value
static inline float gamma_to_linear_sRGB(float c) {
    if (c <= 0.f) {
        return 0.f;
    } else if (c < 0.04045f) {
        return c / 12.92f;
    } else if (c < 1.f) {
        return std::pow((c + 0.055f) / 1.055f, 2.4f);
    }

    return 1.f;
}

// convert sRGB gamma color to sRGB linear color
static inline float3 gamma_to_linear_sRGB(byte3 c) {
    return float3(gamma_to_linear_sRGB(float(c[0]) / 255.f),
                  gamma_to_linear_sRGB(float(c[1]) / 255.f),
                  gamma_to_linear_sRGB(float(c[2]) / 255.f));
}

// convert sRGB gamma color to sRGB linear color
static inline float3 gamma_to_linear_sRGB(float3 const& c) {
    return float3(gamma_to_linear_sRGB(c[0]), gamma_to_linear_sRGB(c[1]),
                  gamma_to_linear_sRGB(c[1]));
}

static inline float4 gamma_to_linear_sRGB(byte4 c) {
    return float4(gamma_to_linear_sRGB(float(c[0]) / 255.f),
                  gamma_to_linear_sRGB(float(c[1]) / 255.f),
                  gamma_to_linear_sRGB(float(c[2]) / 255.f), float(c[3]) / 255.f);
}

// convert linear color to gamma color
static inline float3 linear_to_gamma(float3 const& c, float gamma) {
    float const p = 1.f / gamma;

    return float3(std::pow(c[0], p), std::pow(c[1], p), std::pow(c[2], p));
}

// convert gamma color to linear color
static inline float3 gamma_to_linear(float3 const& c, float gamma) {
    return float3(std::pow(c[0], gamma), std::pow(c[1], gamma), std::pow(c[2], gamma));
}

// XYZ <-> RGB conversion matrices
// http://www.brucelindbloom.com/index.html?Eqn_RGB_XYZ_Matrix.html

static inline float constexpr luminance(float3 const& c) {
    return 0.2126729f * c[0] + 0.7151522f * c[1] + 0.0721750f * c[2];
}

float constexpr CIE_constant = 683.002f;

static inline float constexpr watt_to_lumen(float3 const& c) {
    return CIE_constant * luminance(c);
}

}  // namespace spectrum

#endif
