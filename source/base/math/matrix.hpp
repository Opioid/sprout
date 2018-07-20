#ifndef SU_BASE_MATH_MATRIX_HPP
#define SU_BASE_MATH_MATRIX_HPP

namespace math {

struct alignas(16) Matrix3x3f_a;
struct alignas(16) Matrix4x4f_a;

using VMatrix3x3f_a = const Matrix3x3f_a;  //&;
using VMatrix4x4f_a = const Matrix4x4f_a;  //&;

// template<typename T> struct Matrix3x3;

}  // namespace math

// using float3x3 = math::Matrix3x3<float>;
using float3x3 = math::Matrix3x3f_a;
using float4x4 = math::Matrix4x4f_a;

using v_float3x3 = math::VMatrix3x3f_a;
using v_float4x4 = math::VMatrix4x4f_a;

#endif
