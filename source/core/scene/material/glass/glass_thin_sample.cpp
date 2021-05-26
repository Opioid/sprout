#include "glass_thin_sample.hpp"
#include "base/math/vector3.inl"
#include "rendering/integrator/integrator_helper.hpp"
#include "sampler/sampler.hpp"
#include "scene/material/bxdf.hpp"
#include "scene/material/fresnel/fresnel.inl"
#include "scene/material/material_sample.inl"

#include "base/debug/assert.hpp"
#include "scene/material/material_test.hpp"

namespace scene::material::glass {

static void reflect(float3_p wo, float3_p n, float n_dot_wo, bxdf::Sample& result);

static void refract(float3_p wo, float3_p color, bxdf::Sample& result);

Sample_thin::Sample_thin() {
    properties_.unset(Property::Can_evaluate);
    properties_.set(Property::Translucent);
}

bxdf::Result Sample_thin::evaluate(float3_p /*wi*/) const {
    return {float3(0.f), 0.f};
}

void Sample_thin::sample(Sampler& sampler, RNG& rng, bxdf::Sample& result) const {
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

    float const p = sampler.sample_1D(rng);

    if (p <= f) {
        reflect(wo_, n, n_dot_wo, result);
    } else {
        float const n_dot_wi        = clamp(n_dot_wo);
        float const approx_distance = thickness_ / n_dot_wi;

        float3 const attenuation = rendering::attenuation(approx_distance, absorption_coef_);

        refract(wo_, attenuation, result);
    }

    result.wavelength = 0.f;
}

void Sample_thin::set(float3_p absorption_coef, float ior, float ior_outside, float thickness) {
    absorption_coef_ = absorption_coef;
    ior_             = ior;
    ior_outside_     = ior_outside;
    thickness_       = thickness;
}

void reflect(float3_p wo, float3_p n, float n_dot_wo, bxdf::Sample& result) {
    result.reflection = float3(1.f);
    result.wi         = normalize(2.f * n_dot_wo * n - wo);
    result.pdf        = 1.f;
    result.type.clear(bxdf::Type::Specular_reflection);

    //    SOFT_ASSERT(testing::check(result, sample.wo_, layer));
}

void refract(float3_p wo, float3_p color, bxdf::Sample& result) {
    result.reflection = color;
    result.wi         = -wo;
    result.pdf        = 1.f;
    result.type.clear(bxdf::Type::Straight);

    //    SOFT_ASSERT(testing::check(result, sample.wo_, layer));
}

}  // namespace scene::material::glass
