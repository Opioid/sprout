#ifndef SU_BASE_MATH_SAMPLING_SAMPLING_HPP
#define SU_BASE_MATH_SAMPLING_SAMPLING_HPP

#include "math/math.hpp"
#include "math/matrix3x3.inl"
#include "math/sincos.hpp"
#include "math/vector3.inl"

namespace math {

static inline float2 sample_disk_concentric(float2 uv) noexcept {
    float2 const s = 2.f * uv - 1.f;

    if (0.f == s[0] && 0.f == s[1]) {
        return float2(0.f);
    }

    float r;
    float theta;

    if (std::abs(s[0]) > std::abs(s[1])) {
        r     = s[0];
        theta = (Pi / 4.f) * (s[1] / s[0]);
    } else {
        r     = s[1];
        theta = (Pi / 2.f) - (Pi / 4.f) * (s[0] / s[1]);
    }

    auto const [sin_theta, cos_theta] = sincos(theta);

    return float2(cos_theta * r, sin_theta * r);
}

static inline float3 sample_oriented_disk_concentric(float2 uv, float3 const& x,
                                                     float3 const& y) noexcept {
    float2 const d = sample_disk_concentric(uv);

    return d[0] * x + d[1] * y;
}

static inline float2 sample_triangle_uniform(float2 uv) noexcept {
    float const su = std::sqrt(uv[0]);

    return float2(1.f - su, uv[1] * su);
}

static inline float2 sample_triangle_uniform(float u) noexcept {
    uint32_t const ui = static_cast<uint32_t>(u * (1ull << 32));

    float2 a(1.f, 0.f);
    float2 b(0.f, 1.f);
    float2 c(0.f, 0.f);

    for (int32_t i = 0; i < 16; ++i) {
        int32_t d = (ui >> (2 * (15 - i))) & 0x3ull;

        float2 An, Bn, Cn;
        switch (d) {
            case 0:
                An = (b + c) / 2.f;
                Bn = (a + c) / 2.f;
                Cn = (a + b) / 2.f;
                break;
            case 1:
                An = a;
                Bn = (a + b) / 2.f;
                Cn = (a + c) / 2.f;
                break;
            case 2:
                An = (b + a) / 2.f;
                Bn = b;
                Cn = (b + c) / 2.f;
                break;
            case 3:
                An = (c + a) / 2.f;
                Bn = (c + b) / 2.f;
                Cn = c;
                break;
        }
        a = An;
        b = Bn;
        c = Cn;
    }

    return (a + b + c) / 3.f;
}

static inline float3 sample_hemisphere_uniform(float2 uv) noexcept {
    float const z   = 1.f - uv[0];
    float const r   = std::sqrt(1.f - z * z);
    float const phi = uv[1] * (2.f * Pi);

    auto const [sin_phi, cos_phi] = sincos(phi);

    return float3(cos_phi * r, sin_phi * r, z);
}

static inline float3 sample_oriented_hemisphere_uniform(float2 uv, float3 const& x, float3 const& y,
                                                        float3 const& z) noexcept {
    float const za  = 1.f - uv[0];
    float const r   = std::sqrt(1.f - za * za);
    float const phi = uv[1] * (2.f * Pi);

    auto const [sin_phi, cos_phi] = sincos(phi);

    return (cos_phi * r) * x + (sin_phi * r) * y + za * z;
}

static inline float3 sample_oriented_hemisphere_uniform(float2 uv, float3x3 const& m) noexcept {
    float const za  = 1.f - uv[0];
    float const r   = std::sqrt(1.f - za * za);
    float const phi = uv[1] * (2.f * Pi);

    auto const [sin_phi, cos_phi] = sincos(phi);

    return (cos_phi * r) * m.r[0] + (sin_phi * r) * m.r[1] + za * m.r[2];
}

static inline float3 sample_hemisphere_cosine(float2 uv) noexcept {
    float2 const xy = sample_disk_concentric(uv);
    float const  z  = std::sqrt(std::max(0.f, 1.f - xy[0] * xy[0] - xy[1] * xy[1]));

    return float3(xy[0], xy[1], z);
}

static inline float3 sample_oriented_hemisphere_cosine(float2 uv, float3x3 const& m) noexcept {
    float2 const xy = sample_disk_concentric(uv);
    float const  za = std::sqrt(std::max(0.f, 1.f - xy[0] * xy[0] - xy[1] * xy[1]));

    return xy[0] * m.r[0] + xy[1] * m.r[1] + za * m.r[2];
}

static inline float3 sample_oriented_hemisphere_cosine(float2 uv, float3 const& x, float3 const& y,
                                                       float3 const& z) noexcept {
    float2 const xy = sample_disk_concentric(uv);
    float const  za = std::sqrt(std::max(0.f, 1.f - xy[0] * xy[0] - xy[1] * xy[1]));

    return xy[0] * x + xy[1] * y + za * z;
}

static inline float3 sample_sphere_uniform(float2 uv) noexcept {
    float const z   = 1.f - 2.f * uv[0];
    float const r   = std::sqrt(std::max(0.f, 1.f - z * z));
    float const phi = uv[1] * (2.f * Pi);

    auto const [sin_phi, cos_phi] = sincos(phi);

    return float3(cos_phi * r, sin_phi * r, z);
}

static inline float3 sample_sphere_volume_uniform(float3 const& uvw) noexcept {
    //    var u = Math.random();
    //    var v = Math.random();
    //    var theta = u * 2.0 * Math.PI;
    //    var phi = Math.acos(2.0 * v - 1.0);
    //    var r = Math.cbrt(Math.random());
    //    var sinTheta = Math.sin(theta);
    //    var cosTheta = Math.cos(theta);
    //    var sinPhi = Math.sin(phi);
    //    var cosPhi = Math.cos(phi);
    //    var x = r * sinPhi * cosTheta;
    //    var y = r * sinPhi * sinTheta;
    //    var z = r * cosPhi;

    float const theta = uvw[0] * (2.f * Pi);
    float const phi   = std::acos(2.f * uvw[1] - 1.f);
    float const r     = std::cbrt(uvw[2]);

    auto const [sin_theta, cos_theta] = sincos(theta);
    auto const [sin_phi, cos_phi]     = sincos(phi);

    return float3(r * sin_phi * cos_theta, r * sin_phi * sin_theta, r * cos_phi);
}

static inline float3 sphere_direction(float sin_theta, float cos_theta, float phi, float3 const& x,
                                      float3 const& y, float3 const& z) noexcept {
    auto const [sin_phi, cos_phi] = sincos(phi);

    return sin_theta * cos_phi * x + sin_theta * sin_phi * y + cos_theta * z;
}

static inline float3 sample_oriented_cone_uniform(float2 uv, float cos_theta_max, float3 const& x,
                                                  float3 const& y, float3 const& z) noexcept {
    float const cos_theta = (1.f - uv[0]) + uv[0] * cos_theta_max;
    float const sin_theta = std::sqrt(1.f - cos_theta * cos_theta);
    float const phi       = uv[1] * (2.f * Pi);

    auto const [sin_phi, cos_phi] = sincos(phi);

    return (cos_phi * sin_theta) * x + (sin_phi * sin_theta) * y + cos_theta * z;
}

static inline float cone_pdf_uniform(float cos_theta_max) noexcept {
    return 1.f / ((2.f * Pi) * (1.f - cos_theta_max));
}

}  // namespace math

#endif
