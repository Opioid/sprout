#pragma once

#include "math/vector.hpp"

namespace spectrum {

float3 XYZ_to_linear_RGB(float3_p xyz);

constexpr uint32_t CIE_XYZ_Num = (830 - 360) + 1; // 471

extern const float CIE_X_360_830_1nm[CIE_XYZ_Num];
extern const float CIE_Y_360_830_1nm[CIE_XYZ_Num];
extern const float CIE_Z_360_830_1nm[CIE_XYZ_Num];
extern const float CIE_Wavelengths_360_830_1nm[CIE_XYZ_Num];

}
