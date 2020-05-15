#ifndef SU_BASE_SPECTRUM_ACES_HPP
#define SU_BASE_SPECTRUM_ACES_HPP

#include "math/vector3.inl"

namespace spectrum {

// https://www.shadertoy.com/view/WltSRB

// sRGB => XYZ => D65_2_D60 => AP1
static inline float3 constexpr linear_sRGB_to_AP1(float3 const& srgb) {
    return float3(0.613097f * srgb[0] + 0.339523f * srgb[1] + 0.047379f * srgb[2],
                  0.070194f * srgb[0] + 0.916354f * srgb[1] + 0.013452f * srgb[2],
                  0.020616f * srgb[0] + 0.109570f * srgb[1] + 0.869815f * srgb[2]);
}

static inline float3 constexpr AP1_to_sRGB(float3 const& srgb) {
    return float3(1.70505155f * srgb[0] - 0.62179068 * srgb[1] - 0.0832584f * srgb[2],
                  -0.13025714f * srgb[0] + 1.14080289f * srgb[1] - 0.01054853f * srgb[2],
                  -0.02400328f * srgb[0] - 0.12896877f * srgb[1] + 1.15297171f * srgb[2]);
}

static inline float3 constexpr AP1_to_RRT_SAT(float3 const& acescg) {
    return float3(0.970889f * acescg[0] + 0.026963f * acescg[1] + 0.002148f * acescg[2],
                  0.010889f * acescg[0] + 0.986963f * acescg[1] + 0.002148f * acescg[2],
                  0.010889f * acescg[0] + 0.026963f * acescg[1] + 0.962148f * acescg[2]);
}

// ODT_SAT => XYZ => D60_2_D65 => sRGB
static inline float3 constexpr ODT_SAT_to_sRGB(float3 const& odt) {
    return float3(+1.60475f * odt[0] - 0.53108f * odt[1] - 0.07367f * odt[2],
                  -0.10208f * odt[0] + 1.10813f * odt[1] - 0.00605f * odt[2],
                  -0.00327f * odt[0] - 0.07276f * odt[1] + 1.07602f * odt[2]);
}

static inline float3 constexpr RRT_and_ODT(float3 const& x) {
    float3 const a = x * (x + 0.0245786f) - 0.000090537f;
    float3 const b = x * (0.983729f * x + 0.4329510f) + 0.238081f;
    return a / b;
}

static inline float3 constexpr ToneTF2(float3 const& x) {
    float3 const a = x * (x + 0.0822192);
    float3 const b = x * (0.983521 * x + 0.5001330) + 0.274064;

    return a / b;
}

static inline float3 XYZ_to_AP1(float3 const xyz) {
    return float3(1.6410233797f * xyz[0] + -0.3248032942f * xyz[1] + -0.2364246952f * xyz[2],
                  -0.6636628587f * xyz[0] + 1.6153315917f * xyz[1] + 0.0167563477f * xyz[2],
                  0.0117218943f * xyz[0] + -0.0082844420f * xyz[1] + 0.9883948585f * xyz[2]);
}

static inline float3 XYZ_to_AP1_with_E_to_ACES_W(float3 const xyz) {
    return float3(1.69081621f * xyz[0] + -0.28188793f * xyz[1] + -0.24695759f * xyz[2],
                  -0.6376096f * xyz[0] + 1.56038832f * xyz[1] + 0.01532285f * xyz[2],
                  0.01231807f * xyz[0] + -0.00982717f * xyz[1] + 0.98155884f * xyz[2]);
}

}  // namespace spectrum

#endif
