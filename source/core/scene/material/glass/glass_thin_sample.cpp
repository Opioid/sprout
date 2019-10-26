#include "glass_thin_sample.hpp"
#include "base/math/math.hpp"
#include "base/math/vector3.inl"
#include "rendering/integrator/integrator_helper.hpp"
#include "sampler/sampler.hpp"
#include "scene/material/bxdf.hpp"
#include "scene/material/fresnel/fresnel.inl"
#include "scene/material/material_sample.inl"

#include "base/debug/assert.hpp"
#include "scene/material/material_test.hpp"

namespace scene::material::glass {

float3 const& Sample_thin::base_shading_normal() const noexcept {
    return layer_.n_;
}

float3 Sample_thin::base_tangent_to_world(float3 const& v) const noexcept {
    return layer_.tangent_to_world(v);
}

bxdf::Result Sample_thin::evaluate_f(float3 const& /*wi*/, bool /*include_back*/) const noexcept {
    return {float3(0.f), 0.f};
}

bxdf::Result Sample_thin::evaluate_b(float3 const& /*wi*/, bool /*include_back*/) const noexcept {
    return {float3(0.f), 0.f};
}

void Sample_thin::sample(sampler::Sampler& sampler, bxdf::Sample& result) const noexcept {
    // Thin material is always double sided, so no need to check hemisphere.

    float3 const n = layer_.n_;

    float const eta_i = ior_outside_;
    float const eta_t = ior_;

    float const n_dot_wo = std::min(std::abs(dot(n, wo_)), 1.f);
    float const eta      = eta_i / eta_t;
    float const sint2    = (eta * eta) * (1.f - n_dot_wo * n_dot_wo);

    float f;
    if (sint2 >= 1.f) {
        f = 1.f;
    } else {
        float const n_dot_t = std::sqrt(1.f - sint2);

        f = fresnel::dielectric(n_dot_wo, n_dot_t, eta_i, eta_t);
    }

    float const p = sampler.generate_sample_1D();

    if (p <= f) {
        reflect(wo_, n, n_dot_wo, result);
    } else {
        float const n_dot_wi = clamp(n_dot_wo);

        float const approximated_distance = thickness_ / n_dot_wi;

        float3 const attenuation = rendering::attenuation(approximated_distance,
                                                          absorption_coefficient_);

        refract(wo_, attenuation * color_, result);
    }

    result.wavelength = 0.f;
}

bool Sample_thin::is_translucent() const noexcept {
    return true;
}

void Sample_thin::set(float3 const& refraction_color, float3 const& absorption_coefficient,
                      float ior, float ior_outside, float thickness) noexcept {
    color_                  = refraction_color;
    absorption_coefficient_ = absorption_coefficient;
    ior_                    = ior;
    ior_outside_            = ior_outside;
    thickness_              = thickness;
}

void Sample_thin::reflect(float3 const& wo, float3 const& n, float n_dot_wo,
                          bxdf::Sample& result) noexcept {
    result.reflection = float3(1.f);
    result.wi         = normalize(2.f * n_dot_wo * n - wo);
    result.pdf        = 1.f;
    result.type.clear(bxdf::Type::Specular_reflection);

    //    SOFT_ASSERT(testing::check(result, sample.wo_, layer));
}

void Sample_thin::refract(float3 const& wo, float3 const& color, bxdf::Sample& result) noexcept {
    result.reflection = color;
    result.wi         = -wo;
    result.pdf        = 1.f;
    result.type.clear(bxdf::Type::Straight);

    //    SOFT_ASSERT(testing::check(result, sample.wo_, layer));
}

}  // namespace scene::material::glass
