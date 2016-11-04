#pragma once

#include "xyz.hpp"
#include "math/vector.inl"

namespace spectrum {

// XYZ <-> RGB conversion matrices
// http://www.brucelindbloom.com/index.html?Eqn_RGB_XYZ_Matrix.html

// This function uses CIE-RGB with illuminant E

inline float3 XYZ_to_linear_RGB(float3_p xyz) {
	return float3(
		2.3706743f * xyz.x - 0.9000405f * xyz.y - 0.4706338f * xyz.z,
	   -0.5138850f * xyz.x + 1.4253036f * xyz.y + 0.0885814f * xyz.z,
		0.0052982f * xyz.x - 0.0146949f * xyz.y + 1.0093968f * xyz.z);
}

}
