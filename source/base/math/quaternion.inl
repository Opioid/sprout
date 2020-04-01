#ifndef SU_BASE_MATH_QUATERNION_INL
#define SU_BASE_MATH_QUATERNION_INL

#include "matrix3x3.inl"
#include "quaternion.hpp"

namespace math {

/****************************************************************************
 *
 * Aligned quaternion functions
 *
 ****************************************************************************/

namespace quaternion {

static inline Quaternion create(float3x3 const& m) {
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

// Converting a Rotation Matrix to a Quaternion
// https://d3cw3dd2w32x2b.cloudfront.net/wp-content/uploads/2015/01/matrix-to-quat.pdf
/*
static inline Quaternion create_also(float3x3 const& m) {
    float t;
    Quaternion q;

    if (m.r[2][2] < 0.f) {
        if (m.r[0][0] > m.r[1][1]) {
            t = 1.f + m.r[0][0] - m.r[1][1] - m.r[2][2];
            q = Quaternion(t, m.r[0][1] + m.r[1][0], m.r[2][0] + m.r[0][2], m.r[1][2] - m.r[2][1]);
        } else {
            t = 1.f - m.r[0][0] + m.r[1][1] - m.r[2][2];
            q = Quaternion(m.r[0][1] + m.r[1][0], t, m.r[1][2] + m.r[2][1], m.r[2][0] - m.r[0][2]);
        }
    } else {
        if (m.r[0][0] < -m.r[1][1]) {
            t = 1.f - m.r[0][0] - m.r[1][1] + m.r[2][2];
            q = Quaternion(m.r[2][0] + m.r[0][2], m.r[1][2] + m.r[2][1], t, m.r[0][1] - m.r[1][0]);
        } else {
            t = 1.f + m.r[0][0] + m.r[1][1] + m.r[2][2];
            q = Quaternion(m.r[1][2] - m.r[2][1], m.r[2][0] - m.r[0][2], m.r[0][1] - m.r[1][0], t);
        }
    }

    return (0.5f / std::sqrt(t)) * q;
}
*/
static inline float3x3 create_matrix3x3(Quaternion const& q) {
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

// https://github.com/erwincoumans/sce_vectormath/blob/master/include/vectormath/scalar/cpp/mat_aos.h
/*
static inline float3x3 create_matrix3x3(Quaternion const& q) {
    float const qx = q[0];
    float const qy = q[1];
    float const qz = q[2];
    float const qw = q[3];
    float const qx2 =  qx + qx ;
    float const qy2 = qy + qy ;
    float const qz2 =  qz + qz ;
    float const qxqx2 =  qx * qx2 ;
    float const qxqy2 =  qx * qy2 ;
    float const qxqz2 =  qx * qz2 ;
    float const qxqw2 =  qw * qx2 ;
    float const qyqy2 =  qy * qy2 ;
    float const qyqz2 =  qy * qz2 ;
    float const qyqw2 =  qw * qy2 ;
    float const qzqz2 =  qz * qz2 ;
    float const qzqw2 =  qw * qz2 ;

    return float3x3(
      ( 1.f - qyqy2 ) - qzqz2 ,  qxqy2 - qzqw2 ,  qxqz2 + qyqw2  ,
      qxqy2 + qzqw2 ,  ( 1.f - qxqx2 ) - qzqz2 ,  qyqz2 - qxqw2  ,
      qxqz2 - qyqw2 ,  qyqz2 + qxqw2 ,  ( 1.f - qxqx2 ) - qyqy2  );
}
*/

static inline Quaternion create_rotation_x(float a) {
    return Quaternion(std::sin(a * 0.5f), 0.f, 0.f, std::cos(a * 0.5f));
}

static inline Quaternion create_rotation_y(float a) {
    return Quaternion(0.f, std::sin(a * 0.5f), 0.f, std::cos(a * 0.5f));
}

static inline Quaternion create_rotation_z(float a) {
    return Quaternion(0.f, 0.f, std::sin(a * 0.5f), std::cos(a * 0.5f));
}

static inline Quaternion mul(Quaternion const& a, Quaternion const& b) {
    return Quaternion((a[3] * b[0] + a[0] * b[3]) + (a[1] * b[2] - a[2] * b[1]),
                      (a[3] * b[1] + a[1] * b[3]) + (a[2] * b[0] - a[0] * b[2]),
                      (a[3] * b[2] + a[2] * b[3]) + (a[0] * b[1] - a[1] * b[0]),
                      (a[3] * b[3] - a[0] * b[0]) - (a[1] * b[1] + a[2] * b[2]));
}

static inline Quaternion slerp(Quaternion const& a, Quaternion const& b, float t) {
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

static inline constexpr Quaternion identity() {
    return Quaternion(0.f, 0.f, 0.f, 1.f);
}

}  // namespace quaternion
}  // namespace math

#endif
