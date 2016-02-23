#pragma once

#include "math.hpp"
#include "vector2.hpp"
#include "vector3.hpp"
#include "vector4.hpp"

namespace math {

typedef Vector2<float>	  float2;
typedef Vector2<int32_t>  int2;
typedef Vector2<uint32_t> uint2;
//typedef Vector3<float>    vec3;
typedef Vector3f_a		  vec3;
typedef Vector3<uint32_t> uint3;
//typedef Vector4<float>    float4;
typedef Vector4f_a		  float4;
typedef Vector4<int32_t>  int4;

typedef FVector3f_a pvec3;

SU_GLOBALCONST(Vector3f_a) vec3_identity(0.f, 0.f, 0.f);
SU_GLOBALCONST(Vector4f_a) float4_identity(0.f, 0.f, 0.f, 0.f);

}
