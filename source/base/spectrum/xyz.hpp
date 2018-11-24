#ifndef SU_BASE_SPECTRUM_XYZ_HPP
#define SU_BASE_SPECTRUM_XYZ_HPP

#include "math/vector3.inl"

// XYZ <-> RGB conversion matrices
// http://www.brucelindbloom.com/index.html?Eqn_RGB_XYZ_Matrix.html

// This function uses CIE-RGB with illuminant E

namespace spectrum {

static inline float3 constexpr XYZ_to_linear_RGB(float3 const& xyz) noexcept {
    return float3(2.3706743f * xyz[0] - 0.9000405f * xyz[1] - 0.4706338f * xyz[2],
                  -0.5138850f * xyz[0] + 1.4253036f * xyz[1] + 0.0885814f * xyz[2],
                  0.0052982f * xyz[0] - 0.0146949f * xyz[1] + 1.0093968f * xyz[2]);
}

// This function uses sRGB with illuminant D65

static inline float3 constexpr XYZ_to_linear_sRGB_D65(float3 const& xyz) noexcept {
    return float3(3.2404542f * xyz[0] - 1.5371385f * xyz[1] - 0.4985314f * xyz[2],
                  -0.9692660f * xyz[0] + 1.8760108f * xyz[1] + 0.0415560f * xyz[2],
                  0.0556434f * xyz[0] - 0.2040259f * xyz[1] + 1.0572252f * xyz[2]);
}

uint32_t constexpr CIE_XYZ_Num = (830 - 360) + 1;  // 471

extern float const CIE_X_360_830_1nm[CIE_XYZ_Num];
extern float const CIE_Y_360_830_1nm[CIE_XYZ_Num];
extern float const CIE_Z_360_830_1nm[CIE_XYZ_Num];
extern float const CIE_Wavelengths_360_830_1nm[CIE_XYZ_Num];

}  // namespace spectrum

#endif
