#ifndef SU_BASE_MATH_MATRIX4X4_INL
#define SU_BASE_MATH_MATRIX4X4_INL

#include "matrix3x3.inl"
#include "matrix4x4.hpp"
#include "quaternion.inl"
#include "transformation.hpp"

namespace math {

inline Matrix4x4f_a::Matrix4x4f_a() = default;

inline Matrix4x4f_a::Matrix4x4f_a(float m00, float m01, float m02, float m03, float m10, float m11,
                                  float m12, float m13, float m20, float m21, float m22, float m23,
                                  float m30, float m31, float m32, float m33)
    : r{{m00, m01, m02, m03}, {m10, m11, m12, m13}, {m20, m21, m22, m23}, {m30, m31, m32, m33}} {}

inline Matrix4x4f_a::Matrix4x4f_a(float const* a)
    : r{{a[0], a[1], a[2], a[3]},
        {a[4], a[5], a[6], a[7]},
        {a[8], a[9], a[10], a[11]},
        {a[12], a[13], a[14], a[15]}} {}

inline Matrix4x4f_a::Matrix4x4f_a(Matrix3x3f_a const& m)
    : r{Vector4f_a(m.r[0]), Vector4f_a(m.r[1]), Vector4f_a(m.r[2]), {0.f, 0.f, 0.f, 1.f}} {}

static inline Matrix4x4f_a compose(Matrix3x3f_a const& basis, Vector3f_a_p scale,
                                   Vector3f_a_p origin) {
    return Matrix4x4f_a(basis.r[0][0] * scale[0], basis.r[0][1] * scale[0],
                        basis.r[0][2] * scale[0], 0.f, basis.r[1][0] * scale[1],
                        basis.r[1][1] * scale[1], basis.r[1][2] * scale[1], 0.f,
                        basis.r[2][0] * scale[2], basis.r[2][1] * scale[2],
                        basis.r[2][2] * scale[2], 0.f, origin[0], origin[1], origin[2], 1.f);
}

static inline Matrix4x4f_a compose(Matrix4x4f_a const& basis, Vector3f_a_p scale,
                                   Vector3f_a_p origin) {
    return Matrix4x4f_a(basis.r[0][0] * scale[0], basis.r[0][1] * scale[0],
                        basis.r[0][2] * scale[0], 0.f, basis.r[1][0] * scale[1],
                        basis.r[1][1] * scale[1], basis.r[1][2] * scale[1], 0.f,
                        basis.r[2][0] * scale[2], basis.r[2][1] * scale[2],
                        basis.r[2][2] * scale[2], 0.f, origin[0], origin[1], origin[2], 1.f);
}

static inline void decompose(Matrix4x4f_a const& m, Matrix3x3f_a& basis, Vector3f_a& scale,
                             Vector3f_a& origin) {
    float3 const s(length(m.x()), length(m.y()), length(m.z()));

    basis  = Matrix3x3f_a(m.x() / s[0], m.y() / s[1], m.z() / s[2]);
    scale  = s;
    origin = m.w();
}

inline Matrix4x4f_a::Matrix4x4f_a(Transformation const& t)
    : Matrix4x4f_a(compose(quaternion::create_matrix3x3(t.rotation), t.scale, t.position)) {}

inline Vector3f_a Matrix4x4f_a::x() const {
    return r[0].xyz();
}

inline Vector3f_a Matrix4x4f_a::y() const {
    return r[1].xyz();
}

inline Vector3f_a Matrix4x4f_a::z() const {
    return r[2].xyz();
}

inline Vector3f_a Matrix4x4f_a::w() const {
    return r[3].xyz();
}

static inline Matrix4x4f_a operator*(Matrix4x4f_a const& a, Matrix4x4f_a const& b) {
    return Matrix4x4f_a((a.r[0][0] * b.r[0][0] + a.r[0][1] * b.r[1][0]) +
                            (a.r[0][2] * b.r[2][0] + a.r[0][3] * b.r[3][0]),
                        (a.r[0][0] * b.r[0][1] + a.r[0][1] * b.r[1][1]) +
                            (a.r[0][2] * b.r[2][1] + a.r[0][3] * b.r[3][1]),
                        (a.r[0][0] * b.r[0][2] + a.r[0][1] * b.r[1][2]) +
                            (a.r[0][2] * b.r[2][2] + a.r[0][3] * b.r[3][2]),
                        (a.r[0][0] * b.r[0][3] + a.r[0][1] * b.r[1][3]) +
                            (a.r[0][2] * b.r[2][3] + a.r[0][3] * b.r[3][3]),

                        (a.r[1][0] * b.r[0][0] + a.r[1][1] * b.r[1][0]) +
                            (a.r[1][2] * b.r[2][0] + a.r[1][3] * b.r[3][0]),
                        (a.r[1][0] * b.r[0][1] + a.r[1][1] * b.r[1][1]) +
                            (a.r[1][2] * b.r[2][1] + a.r[1][3] * b.r[3][1]),
                        (a.r[1][0] * b.r[0][2] + a.r[1][1] * b.r[1][2]) +
                            (a.r[1][2] * b.r[2][2] + a.r[1][3] * b.r[3][2]),
                        (a.r[1][0] * b.r[0][3] + a.r[1][1] * b.r[1][3]) +
                            (a.r[1][2] * b.r[2][3] + a.r[1][3] * b.r[3][3]),

                        (a.r[2][0] * b.r[0][0] + a.r[2][1] * b.r[1][0]) +
                            (a.r[2][2] * b.r[2][0] + a.r[2][3] * b.r[3][0]),
                        (a.r[2][0] * b.r[0][1] + a.r[2][1] * b.r[1][1]) +
                            (a.r[2][2] * b.r[2][1] + a.r[2][3] * b.r[3][1]),
                        (a.r[2][0] * b.r[0][2] + a.r[2][1] * b.r[1][2]) +
                            (a.r[2][2] * b.r[2][2] + a.r[2][3] * b.r[3][2]),
                        (a.r[2][0] * b.r[0][3] + a.r[2][1] * b.r[1][3]) +
                            (a.r[2][2] * b.r[2][3] + a.r[2][3] * b.r[3][3]),

                        (a.r[3][0] * b.r[0][0] + a.r[3][1] * b.r[1][0]) +
                            (a.r[3][2] * b.r[2][0] + a.r[3][3] * b.r[3][0]),
                        (a.r[3][0] * b.r[0][1] + a.r[3][1] * b.r[1][1]) +
                            (a.r[3][2] * b.r[2][1] + a.r[3][3] * b.r[3][1]),
                        (a.r[3][0] * b.r[0][2] + a.r[3][1] * b.r[1][2]) +
                            (a.r[3][2] * b.r[2][2] + a.r[3][3] * b.r[3][2]),
                        (a.r[3][0] * b.r[0][3] + a.r[3][1] * b.r[1][3]) +
                            (a.r[3][2] * b.r[2][3] + a.r[3][3] * b.r[3][3]));
}

static inline Vector3f_a transform_vector(Matrix4x4f_a const& m, Vector3f_a_p v) {
    return Vector3f_a(v[0] * m.r[0][0] + v[1] * m.r[1][0] + v[2] * m.r[2][0],
                      v[0] * m.r[0][1] + v[1] * m.r[1][1] + v[2] * m.r[2][1],
                      v[0] * m.r[0][2] + v[1] * m.r[1][2] + v[2] * m.r[2][2]);
}

static inline Vector3f_a transform_vector_transposed(Matrix4x4f_a const& m, Vector3f_a_p v) {
    return Vector3f_a(v[0] * m.r[0][0] + v[1] * m.r[0][1] + v[2] * m.r[0][2],
                      v[0] * m.r[1][0] + v[1] * m.r[1][1] + v[2] * m.r[1][2],
                      v[0] * m.r[2][0] + v[1] * m.r[2][1] + v[2] * m.r[2][2]);
}

static inline Vector3f_a transform_point(Matrix4x4f_a const& m, Vector3f_a_p v) {
    return Vector3f_a((v[0] * m.r[0][0] + v[1] * m.r[1][0]) + (v[2] * m.r[2][0] + m.r[3][0]),
                      (v[0] * m.r[0][1] + v[1] * m.r[1][1]) + (v[2] * m.r[2][1] + m.r[3][1]),
                      (v[0] * m.r[0][2] + v[1] * m.r[1][2]) + (v[2] * m.r[2][2] + m.r[3][2]));
}

static inline Matrix4x4f_a affine_inverted(Matrix4x4f_a const& m) {
    Matrix4x4f_a o;

    float id;

    {
        float const m00_11 = m.r[0][0] * m.r[1][1];
        float const m01_12 = m.r[0][1] * m.r[1][2];
        float const m02_10 = m.r[0][2] * m.r[1][0];
        float const m00_12 = m.r[0][0] * m.r[1][2];
        float const m01_10 = m.r[0][1] * m.r[1][0];
        float const m02_11 = m.r[0][2] * m.r[1][1];

        id = 1.f / ((m00_11 * m.r[2][2] + m01_12 * m.r[2][0] + m02_10 * m.r[2][1]) -
                    (m00_12 * m.r[2][1] + m01_10 * m.r[2][2] + m02_11 * m.r[2][0]));

        o.r[0][2] = (m01_12 - m02_11) * id;
        o.r[1][2] = (m02_10 - m00_12) * id;
        o.r[2][2] = (m00_11 - m01_10) * id;
        o.r[3][2] = ((m00_12 * m.r[3][1] + m01_10 * m.r[3][2] + m02_11 * m.r[3][0]) -
                     (m00_11 * m.r[3][2] + m01_12 * m.r[3][0] + m02_10 * m.r[3][1])) *
                    id;
    }

    {
        float const m11_22 = m.r[1][1] * m.r[2][2];
        float const m12_21 = m.r[1][2] * m.r[2][1];
        float const m12_20 = m.r[1][2] * m.r[2][0];
        float const m10_22 = m.r[1][0] * m.r[2][2];
        float const m10_21 = m.r[1][0] * m.r[2][1];
        float const m11_20 = m.r[1][1] * m.r[2][0];

        o.r[0][0] = (m11_22 - m12_21) * id;
        o.r[1][0] = (m12_20 - m10_22) * id;
        o.r[2][0] = (m10_21 - m11_20) * id;
        o.r[3][0] = ((m10_22 * m.r[3][1] + m11_20 * m.r[3][2] + m12_21 * m.r[3][0]) -
                     (m10_21 * m.r[3][2] + m11_22 * m.r[3][0] + m12_20 * m.r[3][1])) *
                    id;
    }

    {
        float const m02_21 = m.r[0][2] * m.r[2][1];
        float const m01_22 = m.r[0][1] * m.r[2][2];
        float const m00_22 = m.r[0][0] * m.r[2][2];
        float const m02_20 = m.r[0][2] * m.r[2][0];
        float const m01_20 = m.r[0][1] * m.r[2][0];
        float const m00_21 = m.r[0][0] * m.r[2][1];

        o.r[0][1] = (m02_21 - m01_22) * id;
        o.r[1][1] = (m00_22 - m02_20) * id;
        o.r[2][1] = (m01_20 - m00_21) * id;
        o.r[3][1] = ((m00_21 * m.r[3][2] + m01_22 * m.r[3][0] + m02_20 * m.r[3][1]) -
                     (m00_22 * m.r[3][1] + m01_20 * m.r[3][2] + m02_21 * m.r[3][0])) *
                    id;
    }

    {
        o.r[0][3] = 0.f;
        o.r[1][3] = 0.f;
        o.r[2][3] = 0.f;
        o.r[3][3] = 1.f;
    }

    return o;
}

static inline void set_translation(Matrix4x4f_a& m, Vector3f_a_p v) {
    m.r[0][0] = 1.f;
    m.r[0][1] = 0.f;
    m.r[0][2] = 0.f;
    m.r[0][3] = 0.f;
    m.r[1][0] = 0.f;
    m.r[1][1] = 1.f;
    m.r[1][2] = 0.f;
    m.r[1][3] = 0.f;
    m.r[2][0] = 0.f;
    m.r[2][1] = 0.f;
    m.r[2][2] = 1.f;
    m.r[2][3] = 0.f;
    m.r[3][0] = v[0];
    m.r[3][1] = v[1];
    m.r[3][2] = v[2];
    m.r[3][3] = 1.f;
}

static inline void set_rotation_x(Matrix4x4f_a& m, float a) {
    float const c = std::cos(a);
    float const s = std::sin(a);

    m.r[0][0] = 1.f;
    m.r[0][1] = 0.f;
    m.r[0][2] = 0.f;
    m.r[0][3] = 0.f;
    m.r[1][0] = 0.f;
    m.r[1][1] = c;
    m.r[1][2] = -s;
    m.r[1][3] = 0.f;
    m.r[2][0] = 0.f;
    m.r[2][1] = s;
    m.r[2][2] = c;
    m.r[2][3] = 0.f;
    m.r[3][0] = 0.f;
    m.r[3][1] = 0.f;
    m.r[3][2] = 0.f;
    m.r[3][3] = 1.f;
}

static inline void set_rotation_y(Matrix4x4f_a& m, float a) {
    float const c = std::cos(a);
    float const s = std::sin(a);

    m.r[0][0] = c;
    m.r[0][1] = 0.f;
    m.r[0][2] = s;
    m.r[0][3] = 0.f;
    m.r[1][0] = 0.f;
    m.r[1][1] = 1.f;
    m.r[1][2] = 0.f;
    m.r[1][3] = 0.f;
    m.r[2][0] = -s;
    m.r[2][1] = 0.f;
    m.r[2][2] = c;
    m.r[2][3] = 0.f;
    m.r[3][0] = 0.f;
    m.r[3][1] = 0.f;
    m.r[3][2] = 0.f;
    m.r[3][3] = 1.f;
}

static inline void set_view(Matrix4x4f_a& m, Matrix3x3f_a const& basis, Vector3f_a_p eye) {
    m.r[0][0] = basis.r[0][0];
    m.r[0][1] = basis.r[1][0];
    m.r[0][2] = basis.r[2][0];
    m.r[0][3] = 0.f;

    m.r[1][0] = basis.r[0][1];
    m.r[1][1] = basis.r[1][1];
    m.r[1][2] = basis.r[2][1];
    m.r[1][3] = 0.f;

    m.r[2][0] = basis.r[0][2];
    m.r[2][1] = basis.r[1][2];
    m.r[2][2] = basis.r[2][2];
    m.r[2][3] = 0.f;

    m.r[3][0] = -dot(basis.r[0], eye);
    m.r[3][1] = -dot(basis.r[1], eye);
    m.r[3][2] = -dot(basis.r[2], eye);
    m.r[3][3] = 1.f;
}

static inline void set_perspective(Matrix4x4f_a& m, float fov, float ratio) {
    /*
    xScale     0          0               0
    0        yScale       0               0
    0          0       zf/(zf-zn)         1
    0          0       -zn*zf/(zf-zn)     0
    where:
    yScale = cot(fovY/2)
    xScale = aspect ratio * yScale
    */

    float const t = fov * 0.5f;
    float const y = std::cos(t) / std::sin(t);
    float const x = y / ratio;

    m.r[0][0] = x;
    m.r[0][1] = 0.f;
    m.r[0][2] = 0.f;
    m.r[0][3] = 0.f;

    m.r[1][0] = 0.f;
    m.r[1][1] = y;
    m.r[1][2] = 0.f;
    m.r[1][3] = 0.f;

    m.r[2][0] = 0.f;
    m.r[2][1] = 0.f;
    m.r[2][2] = 1.f;
    m.r[2][3] = 1.f;

    m.r[3][0] = 0.f;
    m.r[3][1] = 0.f;
    m.r[3][2] = 0.f;
    m.r[3][3] = 0.f;
}

inline Simd4x4f::Simd4x4f(Matrix4x4f_a const& source)
    : r{Simdf(source.r[0].v), Simdf(source.r[1].v), Simdf(source.r[2].v), Simdf(source.r[3].v)} {}

static inline Simdf transform_vector(Simd4x4f const& m, Simdf_p v) {
    __m128 result = SU_PERMUTE_PS(v.v, _MM_SHUFFLE(0, 0, 0, 0));
    result        = _mm_mul_ps(result, m.r[0].v);
    __m128 temp   = SU_PERMUTE_PS(v.v, _MM_SHUFFLE(1, 1, 1, 1));
    temp          = _mm_mul_ps(temp, m.r[1].v);
    result        = _mm_add_ps(result, temp);
    temp          = SU_PERMUTE_PS(v.v, _MM_SHUFFLE(2, 2, 2, 2));
    temp          = _mm_mul_ps(temp, m.r[2].v);
    result        = _mm_add_ps(result, temp);
    return result;
}

static inline Simdf transform_point(Simd4x4f const& m, Simdf_p v) {
    __m128 result = SU_PERMUTE_PS(v.v, _MM_SHUFFLE(0, 0, 0, 0));
    result        = _mm_mul_ps(result, m.r[0].v);
    __m128 temp   = SU_PERMUTE_PS(v.v, _MM_SHUFFLE(1, 1, 1, 1));
    temp          = _mm_mul_ps(temp, m.r[1].v);
    result        = _mm_add_ps(result, temp);
    temp          = SU_PERMUTE_PS(v.v, _MM_SHUFFLE(2, 2, 2, 2));
    temp          = _mm_mul_ps(temp, m.r[2].v);
    result        = _mm_add_ps(result, temp);
    result        = _mm_add_ps(result, m.r[3].v);
    return result;
}

}  // namespace math

#endif
