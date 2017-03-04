#pragma once

#include "xyz.hpp"
#include "math/vector.inl"

namespace spectrum {

// XYZ <-> RGB conversion matrices
// http://www.brucelindbloom.com/index.html?Eqn_RGB_XYZ_Matrix.html

// This function uses CIE-RGB with illuminant E

inline float3 XYZ_to_linear_RGB(float3_p xyz) {
	return float3(
		2.3706743f * xyz.v[0] - 0.9000405f * xyz.v[1] - 0.4706338f * xyz.v[2],
	   -0.5138850f * xyz.v[0] + 1.4253036f * xyz.v[1] + 0.0885814f * xyz.v[2],
		0.0052982f * xyz.v[0] - 0.0146949f * xyz.v[1] + 1.0093968f * xyz.v[2]);
}

}
