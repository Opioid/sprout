#ifndef SU_BASE_SPECTRUM_ACES_HPP
#define SU_BASE_SPECTRUM_ACES_HPP

#include "math/vector3.inl"

namespace spectrum {

// Some matrices are from the internet, e.g:
// https://www.shadertoy.com/view/WltSRB
// https://github.com/ampas/aces-dev/blob/master/transforms/ctl/README-MATRIX.md
// Others were generated with aces.py

// sRGB => XYZ => D65_2_D60 => AP1
static inline float3 constexpr sRGB_to_AP1(float3_p srgb) {
    return float3(0.61309732f * srgb[0] + 0.33952285f * srgb[1] + 0.04737928f * srgb[2],
                  0.07019422f * srgb[0] + 0.91635557f * srgb[1] + 0.01345259f * srgb[2],
                  0.02061560f * srgb[0] + 0.10956983f * srgb[1] + 0.86981512f * srgb[2]);
}

static inline float3 constexpr AP1_to_sRGB(float3_p srgb) {
    return float3(+1.70505155f * srgb[0] - 0.62179068f * srgb[1] - 0.08325840f * srgb[2],
                  -0.13025714f * srgb[0] + 1.14080289f * srgb[1] - 0.01054853f * srgb[2],
                  -0.02400328f * srgb[0] - 0.12896877f * srgb[1] + 1.15297171f * srgb[2]);
}

static inline float3 constexpr AP1_to_RRT_SAT(float3_p acescg) {
    return float3(0.970889f * acescg[0] + 0.026963f * acescg[1] + 0.002148f * acescg[2],
                  0.010889f * acescg[0] + 0.986963f * acescg[1] + 0.002148f * acescg[2],
                  0.010889f * acescg[0] + 0.026963f * acescg[1] + 0.962148f * acescg[2]);
}

// sRGB => XYZ => D65_2_D60 => AP1 => RRT_SAT
static inline float3 constexpr sRGB_to_RRT_SAT(float3_p srgb) {
    return float3(0.59719f * srgb[0] + 0.35458f * srgb[1] + 0.04823f * srgb[2],
                  0.07600f * srgb[0] + 0.90834f * srgb[1] + 0.01566f * srgb[2],
                  0.02840f * srgb[0] + 0.13383f * srgb[1] + 0.83777f * srgb[2]);
}

// ODT_SAT => XYZ => D60_2_D65 => sRGB
static inline float3 constexpr ODT_SAT_to_sRGB(float3_p odt) {
    return float3(+1.60475f * odt[0] - 0.53108f * odt[1] - 0.07367f * odt[2],
                  -0.10208f * odt[0] + 1.10813f * odt[1] - 0.00605f * odt[2],
                  -0.00327f * odt[0] - 0.07276f * odt[1] + 1.07602f * odt[2]);
}

// https://www.shadertoy.com/view/WltSRB
static inline float3 constexpr RRT_and_ODT(float3_p x) {
    float3 const a = x * (x + 0.0245786f) - 0.000090537f;
    float3 const b = x * (0.983729f * x + 0.4329510f) + 0.238081f;
    return a / b;
}

// XYZ => D65_2_D60 => AP1
static inline float3 XYZ_to_AP1(float3_p xyz) {
    return float3(+1.66058533f * xyz[0] - 0.31529556f * xyz[1] - 0.24150933f * xyz[2],
                  -0.65992606f * xyz[0] + 1.60839147f * xyz[1] + 0.01729859f * xyz[2],
                  +0.00900257f * xyz[0] - 0.00356688f * xyz[1] + 0.91364331f * xyz[2]);
}

}  // namespace spectrum

#endif
