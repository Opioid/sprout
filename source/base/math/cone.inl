#ifndef SU_BASE_MATH_CONE_INL
#define SU_BASE_MATH_CONE_INL

#include "matrix3x3.inl"
#include "vector4.inl"

namespace math::cone {

// static inline float angle_between(float3_p a, float3_p b) {
//    if (dot(a, b) < 0.f) {
//        return Pi - 2.f * std::asin(length(a + b) / 2.f);
//    } else {
//        return 2.f * std::asin(length(a - b) / 2.f);
//    }
//}

static inline float4 merge(float4_p a, float4_p b) {
    if (float4(1.f) == a) {
        return b;
    }

    if (a == b) {
        return a;
    }

    float a_angle = std::acos(a[3]);
    float b_angle = std::acos(b[3]);

    if (b_angle > a_angle) {
        std::swap(a, b);
        std::swap(a_angle, b_angle);
    }

    float const d_angle = std::acos(clamp(dot(a.xyz(), b.xyz()), -1.f, 1.f));

    if (std::min(d_angle + b_angle, Pi) <= a_angle) {
        return a;
    }

    float const o_angle = (a_angle + d_angle + b_angle) / 2.f;

    if (o_angle >= Pi) {
        float4(a.xyz(), -1.f);
    }

    float const r_angle = o_angle - a_angle;

    //    float3 const wr = cross(a.xyz(), b.xyz());

    //    float const wrl = length(wr);

    //    if (wrl < 0.0001f) {
    //        return a;
    //    }

    float3x3 rot;
    set_rotation(rot, normalize(cross(a.xyz(), b.xyz())), r_angle);

    float3 const axis = normalize(transform_vector(rot, a.xyz()));

    return float4(axis, std::cos(o_angle));
}

static inline float4 transform(Matrix3x3f_a const& m, float4_p v) {
    return float4(v[0] * m.r[0][0] + v[1] * m.r[1][0] + v[2] * m.r[2][0],
                  v[0] * m.r[0][1] + v[1] * m.r[1][1] + v[2] * m.r[2][1],
                  v[0] * m.r[0][2] + v[1] * m.r[1][2] + v[2] * m.r[2][2], v[3]);
}

}  // namespace math::cone

#endif
