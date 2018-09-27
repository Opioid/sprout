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

const material::Layer& Sample_thin::base_layer() const noexcept {
    return layer_;
}

bxdf::Result Sample_thin::evaluate(float3 const& /*wi*/) const noexcept {
    return {float3::identity(), 0.f};
}

void Sample_thin::sample(sampler::Sampler& sampler, bxdf::Sample& result) const noexcept {
    float const p = sampler.generate_sample_1D();

    if (p < 0.5f) {
        reflect(sampler, result);
        result.pdf *= 0.5f;
    } else {
        refract(sampler, result);
        result.pdf *= 0.5f;
    }
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

float Sample_thin::reflect(sampler::Sampler& /*sampler*/, bxdf::Sample& result) const noexcept {
    float3 n = layer_.n_;

    float eta_i = 1.f / ior_;
    float eta_t = ior_;

    if (!same_hemisphere(wo_)) {
        n *= -1.f;
        eta_t = eta_i;
        eta_i = ior_;
    }

    float const n_dot_wo = math::saturate(math::dot(n, wo_));

    float sint2 = (eta_i * eta_i) * (1.f - n_dot_wo * n_dot_wo);

    float f;
    if (sint2 > 1.f) {
        f = 1.f;
    } else {
        float n_dot_t = std::sqrt(1.f - sint2);

        // fresnel has to be the same value that would have been computed by BRDF
        f = fresnel::dielectric(n_dot_wo, n_dot_t, eta_i, eta_t);
    }

    result.reflection = float3(f);
    result.wi         = math::normalize(2.f * n_dot_wo * n - wo_);
    result.pdf        = 1.f;
    result.type.clear(bxdf::Type::Specular_reflection);

    //   SOFT_ASSERT(testing::check(result, sample.wo_, layer));

    return 1.f;
}

float Sample_thin::refract(sampler::Sampler& /*sampler*/, bxdf::Sample& result) const noexcept {
    float3 n = layer_.n_;

    float eta_i = 1.f / ior_;
    float eta_t = ior_;

    if (!same_hemisphere(wo_)) {
        n *= -1.f;
        eta_t = eta_i;
        eta_i = ior_;
    }

    float const n_dot_wo = math::saturate(math::dot(n, wo_));

    float sint2 = (eta_i * eta_i) * (1.f - n_dot_wo * n_dot_wo);

    if (sint2 > 1.f) {
        result.pdf = 0.f;
        return 0.f;
    }

    float n_dot_t = std::sqrt(1.f - sint2);

    // fresnel has to be the same value that would have been computed by BRDF
    float f = fresnel::dielectric(n_dot_wo, n_dot_t, eta_i, eta_t);

    float const n_dot_wi = layer_.clamp_n_dot(wo_);

    float const approximated_distance = thickness_ / n_dot_wi;

    float3 attenuation = rendering::attenuation(approximated_distance, absorption_coefficient_);

    result.reflection = (1.f - f) * color_ * attenuation;
    result.wi         = -wo_;
    result.pdf        = 1.f;
    // The integrator should not handle this like a proper transmission.
    result.type.clear(bxdf::Type::Specular_reflection);

    //   SOFT_ASSERT(testing::check(result, sample.wo_, layer));

    return 1.f;
}

}  // namespace scene::material::glass
