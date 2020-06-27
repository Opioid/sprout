#ifndef SU_BASE_SPECTRUM_XYZ_HPP
#define SU_BASE_SPECTRUM_XYZ_HPP

#include "math/vector3.inl"

// XYZ <-> RGB conversion matrix

// http://www.brucelindbloom.com/index.html?Eqn_RGB_XYZ_Matrix.html
// This function uses CIE-RGB with illuminant E

namespace spectrum {

static inline float3 constexpr XYZ_to_linear_RGB(float3 const& xyz) {
    return float3(2.3706743f * xyz[0] - 0.9000405f * xyz[1] - 0.4706338f * xyz[2],
                  -0.5138850f * xyz[0] + 1.4253036f * xyz[1] + 0.0885814f * xyz[2],
                  0.0052982f * xyz[0] - 0.0146949f * xyz[1] + 1.0093968f * xyz[2]);
}

// http://terathon.com/blog/rgb-xyz-conversion-matrix-accuracy/
// This function uses sRGB with illuminant D65

static inline float3 constexpr XYZ_to_sRGB(float3 const& xyz) {
    return float3(3.240970f * xyz[0] - 1.537383f * xyz[1] - 0.498611f * xyz[2],
                  -0.969244f * xyz[0] + 1.875968f * xyz[1] + 0.041555f * xyz[2],
                  0.055630f * xyz[0] - 0.203977f * xyz[1] + 1.056972f * xyz[2]);
}

inline uint32_t constexpr CIE_XYZ_Num = (830 - 360) + 1;  // 471

extern float const CIE_X_360_830_1nm[CIE_XYZ_Num];
extern float const CIE_Y_360_830_1nm[CIE_XYZ_Num];
extern float const CIE_Z_360_830_1nm[CIE_XYZ_Num];
extern float const CIE_Wavelengths_360_830_1nm[CIE_XYZ_Num];

}  // namespace spectrum

#endif
