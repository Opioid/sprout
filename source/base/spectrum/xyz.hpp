#pragma once

#include "math/vector3.inl"

// XYZ <-> RGB conversion matrices
// http://www.brucelindbloom.com/index.html?Eqn_RGB_XYZ_Matrix.html

// This function uses CIE-RGB with illuminant E

namespace spectrum {

static inline float3 XYZ_to_linear_RGB(float3_p xyz) {
	return float3( 2.3706743f * xyz[0] - 0.9000405f * xyz[1] - 0.4706338f * xyz[2],
				  -0.5138850f * xyz[0] + 1.4253036f * xyz[1] + 0.0885814f * xyz[2],
				   0.0052982f * xyz[0] - 0.0146949f * xyz[1] + 1.0093968f * xyz[2]);
}

constexpr uint32_t CIE_XYZ_Num = (830 - 360) + 1; // 471

extern const float CIE_X_360_830_1nm[CIE_XYZ_Num];
extern const float CIE_Y_360_830_1nm[CIE_XYZ_Num];
extern const float CIE_Z_360_830_1nm[CIE_XYZ_Num];
extern const float CIE_Wavelengths_360_830_1nm[CIE_XYZ_Num];

}
