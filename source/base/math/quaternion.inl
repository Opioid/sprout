#ifndef SU_BASE_MATH_QUATERNION_INL
#define SU_BASE_MATH_QUATERNION_INL

#include "matrix3x3.inl"
#include "quaternion.hpp"

namespace math {

/****************************************************************************
 *
 * Generic quaternion
 *
 ****************************************************************************/
/*
template<typename T>
Quaternion<T>::Quaternion() {}

template<typename T>
Quaternion<T>::Quaternion(T x, T y, T z, T w) : x(x), y(y), z(z), w(w) {}

template<typename T>
Quaternion<T>::Quaternion(const Matrix3x3<T>& m) {
        T trace = m.m00 + m.m11 + m.m22;
        T temp[4];

        if (trace > T(0)) {
                T s = std::sqrt(trace + T(1));
                temp[3] = s * T(0.5);
                s = T(0.5) / s;

                temp[0] = (m.m21 - m.m12) * s;
                temp[1] = (m.m02 - m.m20) * s;
                temp[2] = (m.m10 - m.m01) * s;
        } else {
                int i = m.m00 < m.m11 ? (m.m11 < m.m22 ? 2 : 1) : (m.m00 < m.m22 ? 2 : 0);
                int j = (i + 1) % 3;
                int k = (i + 2) % 3;

                T s = std::sqrt(m.m[i * 3 + i] - m.m[j * 3 + j] - m.m[k * 3 + k] + T(1));
                temp[i] = s * T(0.5);
                s = T(0.5) / s;

                temp[3] = (m.m[k * 3 + j] - m.m[j * 3 + k]) * s;
                temp[j] = (m.m[j * 3 + i] + m.m[i * 3 + j]) * s;
                temp[k] = (m.m[k * 3 + i] + m.m[i * 3 + k]) * s;
        }

        x = temp[0];
        y = temp[1];
        z = temp[2];
        w = temp[3];
}

template<typename T>
Quaternion<T> Quaternion<T>::operator*(const Quaternion<T>& q) const {
        return Quaternion<T>(w * q[0] + x * q[3] + y * q[2] - z * q[1],
                                                 w * q[1] + y * q[3] + z * q[0] - x * q[2],
                                                 w * q[2] + z * q[3] + x * q[1] - y * q[0],
                                                 w * q[3] - x * q[0] - y * q[1] - z * q[2]);
}

template<typename T>
const Quaternion<T> Quaternion<T>::identity(T(0), T(0), T(0), T(1));

template<typename T>
T dot(const Quaternion<T>& a, const Quaternion<T>& b) {
        return (a[0] * b[0] + a[1] * b[1]) + (a[2] * b[2] + a[3] * b[3]);
}

template<typename T>
T length(const Quaternion<T>& q) {
        return std::sqrt(dot(q, q));
}

template<typename T>
T angle(const Quaternion<T>& a, const Quaternion<T>& b) {
        T s = std::sqrt(dot(a, a) * dot(b, b));
        return std::acos(dot(a, b) / s);
}

template<typename T>
void set_rotation_x(Quaternion<T>& q, T a) {
        q[0] = std::sin(a * T(0.5));
        q[1] = T(0);
        q[2] = T(0);
        q[3] = std::cos(a * T(0.5));
}

template<typename T>
void set_rotation_y(Quaternion<T>& q, T a) {
        q[0] = T(0);
        q[1] = std::sin(a * T(0.5));
        q[2] = T(0);
        q[3] = std::cos(a * T(0.5));
}

template<typename T>
void set_rotation_z(Quaternion<T>& q, T a) {
        q[0] = T(0);
        q[1] = T(0);
        q[2] = std::sin(a * T(0.5));
        q[3] = std::cos(a * T(0.5));
}

template<typename T>
void set_rotation(Quaternion<T>& q, Vector3<T> const& v, T a) {
        const T d = length(v);

        const T s = std::sin(a * T(0.5)) / d;

        q[0] = v[0] * s;
        q[1] = v[1] * s;
        q[2] = v[2] * s;
        q[3] = cos(a * T(0.5));
}

template<typename T>
void set_rotation(Quaternion<T>& q, T yaw, T pitch, T roll) {
        const T half_yaw   = yaw   * T(0.5);
        const T half_pitch = pitch * T(0.5);
        const T half_roll  = roll  * T(0.5);

        const T cos_yaw   = std::cos(half_yaw);
        const T sin_yaw   = std::sin(half_yaw);
        const T cos_pitch = std::cos(half_pitch);
        const T sin_pitch = std::sin(half_pitch);
        const T cos_roll  = std::cos(half_roll);
        const T sin_roll  = std::sin(half_roll);

        q[0] = cos_roll * sin_pitch * cos_yaw + sin_roll * cos_pitch * sin_yaw;
        q[1] = cos_roll * cos_pitch * sin_yaw - sin_roll * sin_pitch * cos_yaw;
        q[2] = sin_roll * cos_pitch * cos_yaw - cos_roll * sin_pitch * sin_yaw;
        q[3] = cos_roll * cos_pitch * cos_yaw + sin_roll * sin_pitch * sin_yaw;
}

template<typename T>
Quaternion<T> slerp(const Quaternion<T>& a, const Quaternion<T>& b, T t) {
        // calc cosine theta
        T cosom = a[0] * b[0] + a[1] * b[1] + a[2] * b[2] + a[3] * b[3];

        // adjust signs (if necessary)
        Quaternion<T> end = b;

        if (cosom < T(0)) {
                cosom = -cosom;
                end[0] = -end[0];   // Reverse all signs
                end[1] = -end[1];
                end[2] = -end[2];
                end[3] = -end[3];
        }

        // Calculate coefficients
        float sclp;
        float sclq;

        // 0.0001 -> some epsillon
        if (T(1) - cosom > T(0.0001)) {
                // Standard case (slerp)
                float omega = std::acos(cosom); // extract theta from dot product's cos theta
                float sinom = std::sin(omega);
                sclp  = std::sin((T(1) - t) * omega) / sinom;
                sclq  = std::sin(t * omega) / sinom;
        } else {
                // Very close, do linear interpolation (because it's faster)
                sclp = T(1) - t;
                sclq = t;
        }

        return Quaternion<T>(sclp * a[0] + sclq * end[0],
                                                 sclp * a[1] + sclq * end[1],
                                                 sclp * a[2] + sclq * end[2],
                                                 sclp * a[3] + sclq * end[3]);
}
*/
/****************************************************************************
 *
 * Aligned quaternon functions
 *
 ****************************************************************************/

namespace quaternion {

static inline Quaternion create(float3x3 const& m) noexcept {
    float const trace = m.r[0][0] + m.r[1][1] + m.r[2][2];
    Quaternion  temp;

    if (trace > 0.f) {
        float s = std::sqrt(trace + 1.f);
        temp[3] = s * 0.5f;
        s       = 0.5f / s;

        temp[0] = (m.r[2][1] - m.r[1][2]) * s;
        temp[1] = (m.r[0][2] - m.r[2][0]) * s;
        temp[2] = (m.r[1][0] - m.r[0][1]) * s;
    } else {
        uint32_t const i = m.r[0][0] < m.r[1][1] ? (m.r[1][1] < m.r[2][2] ? 2u : 1u)
                                                 : (m.r[0][0] < m.r[2][2] ? 2u : 0u);
        uint32_t const j = (i + 1) % 3;
        uint32_t const k = (i + 2) % 3;

        float s = std::sqrt(m.r[i][i] - m.r[j][j] - m.r[k][k] + 1.f);
        temp[i] = s * 0.5f;
        s       = 0.5f / s;

        temp[3] = (m.r[k][j] - m.r[j][k]) * s;
        temp[j] = (m.r[j][i] + m.r[i][j]) * s;
        temp[k] = (m.r[k][i] + m.r[i][k]) * s;
    }

    return temp;
}

static inline float3x3 create_matrix3x3(Quaternion const& q) noexcept {
    float const d = dot(q, q);
    float const s = 2.f / d;

    float const xs = q[0] * s;
    float const ys = q[1] * s;
    float const zs = q[2] * s;

    float3x3 m;

    {
        float const xx = q[0] * xs;
        float const yy = q[1] * ys;
        float const zz = q[2] * zs;

        m.r[0][0] = 1.f - (yy + zz);
        m.r[1][1] = 1.f - (xx + zz);
        m.r[2][2] = 1.f - (xx + yy);
    }

    {
        float const xy = q[0] * ys;
        float const wz = q[3] * zs;

        m.r[0][1] = xy - wz;
        m.r[1][0] = xy + wz;
    }

    {
        float const xz = q[0] * zs;
        float const wy = q[3] * ys;

        m.r[0][2] = xz + wy;
        m.r[2][0] = xz - wy;
    }

    {
        float const yz = q[1] * zs;
        float const wx = q[3] * xs;

        m.r[1][2] = yz - wx;
        m.r[2][1] = yz + wx;
    }

    return m;
}

static inline Quaternion create_rotation_x(float a) noexcept {
    return Quaternion(std::sin(a * 0.5f), 0.f, 0.f, std::cos(a * 0.5f));
}

static inline Quaternion create_rotation_y(float a) noexcept {
    return Quaternion(0.f, std::sin(a * 0.5f), 0.f, std::cos(a * 0.5f));
}

static inline Quaternion create_rotation_z(float a) noexcept {
    return Quaternion(0.f, 0.f, std::sin(a * 0.5f), std::cos(a * 0.5f));
}

static inline Quaternion mul(Quaternion const& a, Quaternion const& b) noexcept {
    return Quaternion((a[3] * b[0] + a[0] * b[3]) + (a[1] * b[2] - a[2] * b[1]),
                      (a[3] * b[1] + a[1] * b[3]) + (a[2] * b[0] - a[0] * b[2]),
                      (a[3] * b[2] + a[2] * b[3]) + (a[0] * b[1] - a[1] * b[0]),
                      (a[3] * b[3] - a[0] * b[0]) - (a[1] * b[1] + a[2] * b[2]));
}

static inline Quaternion slerp(Quaternion const& a, Quaternion const& b, float t) noexcept {
    // calc cosine theta
    float cosom = (a[0] * b[0] + a[1] * b[1]) + (a[2] * b[2] + a[3] * b[3]);

    // adjust signs (if necessary)
    Quaternion end = b;

    if (cosom < 0.f) {
        cosom  = -cosom;
        end[0] = -end[0];  // Reverse all signs
        end[1] = -end[1];
        end[2] = -end[2];
        end[3] = -end[3];
    }

    // Calculate coefficients
    float sclp;
    float sclq;

    // 0.0001 -> some epsillon
    if (1.f - cosom > 0.0001f) {
        // Standard case (slerp)
        float const omega = std::acos(cosom);  // extract theta from dot product's cos theta
        float const sinom = std::sin(omega);
        sclp              = std::sin((1.f - t) * omega) / sinom;
        sclq              = std::sin(t * omega) / sinom;
    } else {
        // Very close, do linear interpolation (because it's faster)
        sclp = 1.f - t;
        sclq = t;
    }

    return Quaternion(sclp * a[0] + sclq * end[0], sclp * a[1] + sclq * end[1],
                      sclp * a[2] + sclq * end[2], sclp * a[3] + sclq * end[3]);
}

static inline constexpr Quaternion identity() noexcept {
    return Quaternion(0.f, 0.f, 0.f, 1.f);
}

}  // namespace quaternion
}  // namespace math

#endif
