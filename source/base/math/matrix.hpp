#pragma once

#include "matrix3x3.hpp"
#include "matrix4x4.hpp"

namespace math {

// using float3x3 = Matrix3x3<float>;
using float3x3 = Matrix3x3f_a;

// using float4x4 = Matrix4x4<float>;
using float4x4 = Matrix4x4f_a;

}

using float3x3 = math::float3x3;
using float4x4 = math::float4x4;
