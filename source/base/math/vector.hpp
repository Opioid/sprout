#pragma once

#include "math.hpp"
#include "vector2.hpp"
#include "vector3.hpp"
#include "vector4.hpp"

namespace math {

using byte2  = Vector2<uint8_t>;
using float2 = Vector2<float>;
using int2   = Vector2<int32_t>;
using uint2  = Vector2<uint32_t>;

using byte3  = Vector3<uint8_t>;
// using float3 = Vector3<float>;
using float3 = Vector3f_a;
using uint3  = Vector3<uint32_t>;
using packed_float3 = Vector3<float>;

using byte4  = Vector4<uint8_t>;
// using float4 = Vector4<float>;
using float4 = Vector4f_a;
using int4   = Vector4<int32_t>;

using pfloat3 = FVector3f_a;

SU_GLOBALCONST(Vector3f_a) float3_identity(0.f, 0.f, 0.f);
SU_GLOBALCONST(Vector4f_a) float4_identity(0.f, 0.f, 0.f, 0.f);

}
