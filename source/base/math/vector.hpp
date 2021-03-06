#ifndef SU_BASE_MATH_VECTOR_HPP
#define SU_BASE_MATH_VECTOR_HPP

#include <cstdint>

namespace math {

template <typename T>
struct Vector2;

template <typename T>
struct Vector3;

template <typename T>
struct Vector3_a;

struct alignas(16) Vector3f_a;
struct alignas(16) Vector3i_a;

template <typename T>
struct Vector4;
struct alignas(16) Vector4f_a;
struct alignas(16) Vector4i_a;

using Vector3f_a_p = Vector3f_a;
using Vector4f_a_p = Vector4f_a;

struct Scalar;
using Scalar_p = Scalar;

struct Simdf;
using Simdf_p = Simdf;

}  // namespace math

using scalar = math::Scalar;

using byte2   = math::Vector2<uint8_t>;
using ushort2 = math::Vector2<uint16_t>;
using float2  = math::Vector2<float>;
using int2    = math::Vector2<int32_t>;
using uint2   = math::Vector2<uint32_t>;
using ulong2  = math::Vector2<uint64_t>;

using byte3 = math::Vector3<uint8_t>;
// using float3 = math::Vector3<float>;
using float3        = math::Vector3f_a;
using ushort3       = math::Vector3_a<uint16_t>;
using int3          = math::Vector3<int32_t>;
using uint3         = math::Vector3<uint32_t>;
using packed_float3 = math::Vector3<float>;

using byte4   = math::Vector4<uint8_t>;
using ushort4 = math::Vector4<uint16_t>;
// using float4 = math::Vector4<float>;
using float4 = math::Vector4f_a;
using int4   = math::Vector4i_a;

using scalar_p        = scalar;
using float3_p        = float3;         // const&;
using packed_float3_p = packed_float3;  // const&;
using int3_p          = int3;           // const&;
using uint3_p         = uint3;          // const&;
using float4_p        = float4;         // const&;
using int4_p          = int4;           // const&;

using Simdf   = math::Simdf;
using Simdf_p = math::Simdf_p;

using namespace math;

#endif
