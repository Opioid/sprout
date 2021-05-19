#ifndef SU_BASE_MATH_QUATERNION_INL
#define SU_BASE_MATH_QUATERNION_INL

#include "matrix3x3.inl"
#include "quaternion.hpp"

namespace math::quaternion {

// Converting a Rotation Matrix to a Quaternion
// https://d3cw3dd2w32x2b.cloudfront.net/wp-content/uploads/2015/01/matrix-to-quat.pdf

static inline Quaternion create(float3x3 const& m) {
    float      t;
    Quaternion q;

    if (m.r[2][2] < 0.f) {
        if (m.r[0][0] > m.r[1][1]) {
            t = 1.f + m.r[0][0] - m.r[1][1] - m.r[2][2];
            q = Quaternion(t, m.r[0][1] + m.r[1][0], m.r[2][0] + m.r[0][2], m.r[2][1] - m.r[1][2]);
        } else {
            t = 1.f - m.r[0][0] + m.r[1][1] - m.r[2][2];
            q = Quaternion(m.r[0][1] + m.r[1][0], t, m.r[1][2] + m.r[2][1], m.r[0][2] - m.r[2][0]);
        }
    } else {
        if (m.r[0][0] < -m.r[1][1]) {
            t = 1.f - m.r[0][0] - m.r[1][1] + m.r[2][2];
            q = Quaternion(m.r[2][0] + m.r[0][2], m.r[1][2] + m.r[2][1], t, m.r[1][0] - m.r[0][1]);
        } else {
            t = 1.f + m.r[0][0] + m.r[1][1] + m.r[2][2];
            q = Quaternion(m.r[2][1] - m.r[1][2], m.r[0][2] - m.r[2][0], m.r[1][0] - m.r[0][1], t);
        }
    }

    return (0.5f / std::sqrt(t)) * q;
}

static inline Quaternion create(float3_p t, float3_p n) {
    float3 const b = cross(n, t);

    float3x3 const tbn(t, b, n);

    Quaternion q = quaternion::create(tbn);

    static float constexpr Threshold   = 0.000001f;

    static float const renormalization = std::sqrt(1.f - Threshold * Threshold);

    if (std::abs(q[3]) < Threshold) {
        q[0] *= renormalization;
        q[1] *= renormalization;
        q[2] *= renormalization;
        q[3] = q[3] < 0.f ? -Threshold : Threshold;
    }

    if (q[3] < 0.f) {
        q = -q;
    }

    return q;
}

// https://marc-b-reynolds.github.io/quaternions/2017/08/08/QuatRotMatrix.html

static inline float3x3 create_matrix3x3(Quaternion_p q) {
    float const x = q[0];
    float const y = q[1];
    float const z = q[2];
    float const w = q[3];

    float const tx = x + x;
    float const ty = y + y;
    float const tz = z + z;

    float const xx = tx * x;
    float const yy = ty * y;
    float const zz = tz * z;

    float const xy = ty * x;
    float const xz = tz * x;
    float const yz = ty * z;

    float const wx = tx * w;
    float const wy = ty * w;
    float const wz = tz * w;

    return float3x3(1.f - (yy + zz), xy - wz, xz + wy, xy + wz, 1.f - (xx + zz), yz - wx, xz - wy,
                    yz + wx, 1.f - (xx + yy));
}

static inline Vector3f_a_pair create_tangent_normal(Quaternion_p q) {
    float const x  = q[0];
    float const y  = q[1];
    float const z  = q[2];
    float const w  = q[3];
    float const xx = x * x;
    float const yy = y * y;
    float const zz = z * z;
    float const ww = w * w;
    float const tx = x + x;
    float const ty = y + y;
    float const tz = z + z;
    float const xy = ty * x;
    float const xz = tz * x;
    float const yz = ty * z;
    float const wx = tx * w;
    float const wy = ty * w;
    float const wz = tz * w;
    float const t0 = ww - zz;
    float const t1 = xx - yy;

    return { float3(t0 + t1, xy - wz, xz + wy), float3(xz - wy, yz + wx, ww - xx - yy + zz) };
}

static inline float3 create_normal(Quaternion_p q) {
    float const x  = q[0];
    float const y  = q[1];
    float const z  = q[2];
    float const w  = q[3];
    float const xx = x * x;
    float const yy = y * y;
    float const zz = z * z;
    float const ww = w * w;
    float const tx = x + x;
    float const ty = y + y;
    float const tz = z + z;
    float const xz = tz * x;
    float const yz = ty * z;
    float const wx = tx * w;
    float const wy = ty * w;

    return float3(xz - wy, yz + wx, ww - xx - yy + zz);
}

static inline Quaternion create_rotation_x(float a) {
    return Quaternion(std::sin(a * 0.5f), 0.f, 0.f, std::cos(a * 0.5f));
}

static inline Quaternion create_rotation_y(float a) {
    return Quaternion(0.f, std::sin(a * 0.5f), 0.f, std::cos(a * 0.5f));
}

static inline Quaternion create_rotation_z(float a) {
    return Quaternion(0.f, 0.f, std::sin(a * 0.5f), std::cos(a * 0.5f));
}

static inline Quaternion mul(Quaternion_p a, Quaternion_p b) {
    return Quaternion((a[3] * b[0] + a[0] * b[3]) + (a[1] * b[2] - a[2] * b[1]),
                      (a[3] * b[1] + a[1] * b[3]) + (a[2] * b[0] - a[0] * b[2]),
                      (a[3] * b[2] + a[2] * b[3]) + (a[0] * b[1] - a[1] * b[0]),
                      (a[3] * b[3] - a[0] * b[0]) - (a[1] * b[1] + a[2] * b[2]));
}

static inline Quaternion slerp(Quaternion_p a, Quaternion_p b, float t) {
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

        sclp = std::sin((1.f - t) * omega) / sinom;
        sclq = std::sin(t * omega) / sinom;
    } else {
        // Very close, do linear interpolation (because it's faster)
        sclp = 1.f - t;
        sclq = t;
    }

    return Quaternion(sclp * a[0] + sclq * end[0], sclp * a[1] + sclq * end[1],
                      sclp * a[2] + sclq * end[2], sclp * a[3] + sclq * end[3]);
}

inline Quaternion constexpr Identity = {0.f, 0.f, 0.f, 1.f};

}  // namespace math::quaternion

#endif
