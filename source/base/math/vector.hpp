#ifndef SU_BASE_MATH_VECTOR_HPP
#define SU_BASE_MATH_VECTOR_HPP

#include <cstdint>

namespace math {

template <typename T>
struct Vector2;

template <typename T>
struct Vector3;
struct alignas(16) Vector3f_a;
struct alignas(16) Vector3i_a;

template <typename T>
struct Vector4;
struct alignas(16) Vector4f_a;
struct alignas(16) Vector4i_a;

}  // namespace math

using byte2  = math::Vector2<uint8_t>;
using short2 = math::Vector2<int16_t>;
using float2 = math::Vector2<float>;
using int2   = math::Vector2<int32_t>;
using uint2  = math::Vector2<uint32_t>;
using ulong2 = math::Vector2<uint64_t>;

using byte3 = math::Vector3<uint8_t>;

// using float3 = math::Vector3<float>;
using float3 = math::Vector3f_a;

using short3 = math::Vector3<int16_t>;

using int3          = math::Vector3<int32_t>;
using uint3         = math::Vector3<uint32_t>;
using packed_float3 = math::Vector3<float>;

using byte4  = math::Vector4<uint8_t>;
using short4 = math::Vector4<int16_t>;

// using float4 = math::Vector4<float>;
using float4 = math::Vector4f_a;

using int4 = math::Vector4i_a;

using namespace math;

#endif
