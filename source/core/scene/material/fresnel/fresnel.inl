#ifndef SU_CORE_SCENE_MATERIAL_FRESNEL_INL
#define SU_CORE_SCENE_MATERIAL_FRESNEL_INL

#include "base/math/math.hpp"
#include "base/math/vector3.inl"
#include "fresnel.hpp"

namespace scene::material::fresnel {

static inline float schlick_f0(float n0, float n1) noexcept {
    float const t = (n0 - n1) / (n0 + n1);
    return t * t;
}

static inline float schlick(float wo_dot_h, float f0) noexcept {
    return f0 + pow5(1.f - wo_dot_h) * (1.f - f0);

    // Gaussian approximation
    // return f0 + (std::exp2((-5.55473f * wo_dot_h - 6.98316f) * wo_dot_h)) * (1.f - f0);
}

static inline float3 schlick(float wo_dot_h, float3 const& f0) noexcept {
    return f0 + pow5(1.f - wo_dot_h) * (1.f - f0);

    // Gaussian approximation
    // return f0 + (std::exp2((-5.55473f * wo_dot_h - 6.98316f) * wo_dot_h)) * (1.f - f0);
}

static inline float lazanyi_schlick_a(float f0, float f82) noexcept {
    float constexpr cos_theta_max = 1.f / 7.f;

    return (f0 + (1.f - f0) * pow5(1.f - cos_theta_max) - f82) /
           (cos_theta_max * pow6(1.f - cos_theta_max));
}

static inline float3 lazanyi_schlick_a(float3 const& f0, float3 const& f82) noexcept {
    float constexpr cos_theta_max = 1.f / 7.f;

    return (f0 + pow5(1.f - cos_theta_max) * (1.f - f0) - f82) /
           (cos_theta_max * pow6(1.f - cos_theta_max));
}

static inline float3 lazanyi_schlick(float wo_dot_h, float3 const& f0, float3 const& a) noexcept {
    return schlick(wo_dot_h, f0) - wo_dot_h * pow6(1.f - wo_dot_h) * a;
}

static inline float3 conductor(float wo_dot_h, float3 const& eta, float3 const& k) noexcept {
    float3 const tmp_f = eta * eta + k * k;

    float const  wo_dot_h2 = wo_dot_h * wo_dot_h;
    float3 const tmp       = wo_dot_h2 * tmp_f;

    float3 const a   = 2.f * wo_dot_h * eta;
    float3 const r_p = (tmp - a + 1.f) / (tmp + a + 1.f);

    float3 const r_o = (tmp_f - a + wo_dot_h2) / (tmp_f + a + wo_dot_h2);

    return 0.5f * (r_p + r_o);
}

static inline float dielectric(float cos_theta_i, float cos_theta_t, float eta_i,
                               float eta_t) noexcept {
    float const t0 = eta_t * cos_theta_i;
    float const t1 = eta_i * cos_theta_t;

    float const r_p = (t0 - t1) / (t0 + t1);

    float const t2 = eta_i * cos_theta_i;
    float const t3 = eta_t * cos_theta_t;

    float const r_o = (t2 - t3) / (t2 + t3);

    return 0.5f * (r_p * r_p + r_o * r_o);
}

static inline float dielectric_reflect(float cos_theta_i, float eta_i, float eta_t) noexcept {
    float const sin_theta_i = std::sqrt(std::max(0.f, 1.f - cos_theta_i * cos_theta_i));
    float const sin_theta_t = eta_i / eta_t * sin_theta_i;
    float const sint2       = sin_theta_t * sin_theta_t;

    float const cos_theta_t = std::sqrt(1.f - sint2);

    return dielectric(cos_theta_i, cos_theta_t, eta_i, eta_t);
}

// Amplitude reflection coefficient (s-polarized)
static inline float rs(float n1, float n2, float cosI, float cosT) noexcept {
    return (n1 * cosI - n2 * cosT) / (n1 * cosI + n2 * cosT);
}

// Amplitude reflection coefficient (p-polarized)
static inline float rp(float n1, float n2, float cosI, float cosT) noexcept {
    return (n2 * cosI - n1 * cosT) / (n1 * cosT + n2 * cosI);
}

// Amplitude transmission coefficient (s-polarized)
static inline float ts(float n1, float n2, float cosI, float cosT) noexcept {
    return 2.f * n1 * cosI / (n1 * cosI + n2 * cosT);
}

// Amplitude transmission coefficient (p-polarized)
static inline float tp(float n1, float n2, float cosI, float cosT) noexcept {
    return 2.f * n1 * cosI / (n1 * cosT + n2 * cosI);
}

static inline float3 thinfilm(float wo_dot_h, float external_ior, float thinfilm_ior,
                              float internal_ior, float thickness) noexcept {
    // Precompute the reflection phase changes (depends on IOR)
    float delta10 = (thinfilm_ior < external_ior) ? Pi : 0.f;
    float delta12 = (thinfilm_ior < internal_ior) ? Pi : 0.f;
    float delta   = delta10 + delta12;

    // Calculate the thin film layer (and transmitted) angle cosines.
    float a = 1.f - wo_dot_h * wo_dot_h;

    float eta_i = external_ior / internal_ior;
    float eta_t = external_ior / thinfilm_ior;

    float sin1 = eta_t * eta_t * a;
    float sin2 = eta_i * eta_i * a;

    if (sin1 > 1.f || sin2 > 1.f) {
        return float3(1.f);  // Account for TIR.
    }

    float cos1 = std::sqrt(1.f - sin1);
    float cos2 = std::sqrt(1.f - sin2);

    // Calculate the interference phase change.
    float3 phi = float3(2.f * thinfilm_ior * thickness * cos1);
    phi *= (2.f * Pi) / float3(650.f, 510.f, 475.f);
    phi += float3(delta);

    // Obtain the various Fresnel amplitude coefficients.
    float alpha_s = rs(thinfilm_ior, external_ior, cos1, wo_dot_h) *
                    rs(thinfilm_ior, internal_ior, cos1, cos2);
    float alpha_p = rp(thinfilm_ior, external_ior, cos1, wo_dot_h) *
                    rp(thinfilm_ior, internal_ior, cos1, cos2);
    float beta_s = ts(external_ior, thinfilm_ior, wo_dot_h, cos1) *
                   ts(thinfilm_ior, internal_ior, cos1, cos2);
    float beta_p = tp(external_ior, thinfilm_ior, wo_dot_h, cos1) *
                   tp(thinfilm_ior, internal_ior, cos1, cos2);

    // Calculate the s- and p-polarized intensity transmission coefficient
    float3 cos_phi = math::cos(phi);
    float3 ts      = (beta_s * beta_s) / ((alpha_s * alpha_s) - 2.f * alpha_s * cos_phi + 1.f);
    float3 tp      = (beta_p * beta_p) / ((alpha_p * alpha_p) - 2.f * alpha_p * cos_phi + 1.f);

    // Calculate the transmitted power ratio for medium change.
    float beam_ratio = (internal_ior * cos2) / (external_ior * wo_dot_h);

    // Calculate the average reflectance.
    return 1.f - beam_ratio * 0.5f * (ts + tp);
}

inline Schlick1::Schlick1(float f0) noexcept : f0_(f0) {}

inline float Schlick1::operator()(float wo_dot_h) const noexcept {
    return schlick(wo_dot_h, f0_);
}

inline Schlick::Schlick(float f0) noexcept : f0_(f0) {}

inline Schlick::Schlick(float3 const& f0) noexcept : f0_(f0) {}

inline float3 Schlick::operator()(float wo_dot_h) const noexcept {
    return schlick(wo_dot_h, f0_);
}

inline Lazanyi_schlick::Lazanyi_schlick(float f0, float a) noexcept : f0_(f0), a_(a) {}

inline Lazanyi_schlick::Lazanyi_schlick(float3 const& f0, float3 const& a) noexcept
    : f0_(f0), a_(a) {}

inline float3 Lazanyi_schlick::operator()(float wo_dot_h) const noexcept {
    return lazanyi_schlick(wo_dot_h, f0_, a_);
}

inline Thinfilm::Thinfilm(float external_ior, float thinfilm_ior, float internal_ior,
                          float thickness) noexcept
    : external_ior_(external_ior),
      thinfilm_ior_(thinfilm_ior),
      internal_ior_(internal_ior),
      thickness_(thickness) {}

inline float3 Thinfilm::operator()(float wo_dot_h) const noexcept {
    return thinfilm(wo_dot_h, external_ior_, thinfilm_ior_, internal_ior_, thickness_);
}

inline Dielectric::Dielectric(float eta_i, float eta_t) noexcept : eta_i_(eta_i), eta_t_(eta_t) {}

inline float3 Dielectric::operator()(float wo_dot_h) const noexcept {
    return float3(dielectric_reflect(wo_dot_h, eta_i_, eta_t_));
}

inline Conductor::Conductor(float3 const& eta, float3 const& k) noexcept : eta_(eta), k_(k) {}

inline float3 Conductor::operator()(float wo_dot_h) const noexcept {
    return conductor(wo_dot_h, eta_, k_);
}

inline Constant::Constant(float f) noexcept : f_(f) {}

inline Constant::Constant(float3 const& f) noexcept : f_(f) {}

inline float3 Constant::operator()(float /*wo_dot_h*/) const noexcept {
    return f_;
}

}  // namespace scene::material::fresnel

#endif
