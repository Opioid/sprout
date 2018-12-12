#include "glass_sample.hpp"
#include "base/math/math.hpp"
#include "base/math/vector3.inl"
#include "sampler/sampler.hpp"
#include "scene/material/bxdf.hpp"
#include "scene/material/collision_coefficients.inl"
#include "scene/material/fresnel/fresnel.inl"
#include "scene/material/material_sample.inl"

#include "base/debug/assert.hpp"
#include "scene/material/material_test.hpp"

namespace scene::material::glass {

const material::Layer& Sample::base_layer() const noexcept {
    return layer_;
}

bxdf::Result Sample::evaluate(float3 const& /*wi*/, bool /*include_back*/) const noexcept {
    return {float3(0.f), 0.f};
}

void Sample::sample(sampler::Sampler& sampler, bxdf::Sample& result) const noexcept {
    float const p = sampler.generate_sample_1D();

    sample(ior_, p, result);

    result.wavelength = 0.f;
}

void Sample::set(float3 const& refraction_color, float ior, float ior_outside) noexcept {
    color_       = refraction_color;
    ior_         = ior;
    ior_outside_ = ior_outside;
}

void Sample::sample(float ior, float p, bxdf::Sample& result) const noexcept {
    float3 n = layer_.n_;

    float eta_i = ior_outside_;
    float eta_t = ior;

    if (!same_hemisphere(wo_)) {
        n = -n;

        std::swap(eta_i, eta_t);
    }

    float const n_dot_wo = std::min(std::abs(dot(n, wo_)), 1.f);
    float const eta      = eta_i / eta_t;
    float const sint2    = (eta * eta) * (1.f - n_dot_wo * n_dot_wo);

    float n_dot_t;
    float f;
    if (sint2 >= 1.f) {
        n_dot_t = 0.f;

        f = 1.f;
    } else {
        n_dot_t = std::sqrt(1.f - sint2);

        f = fresnel::dielectric(n_dot_wo, n_dot_t, eta_i, eta_t);
    }

    if (p <= f) {
        reflect(wo_, n, n_dot_wo, result);
    } else {
        refract(wo_, n, color_, n_dot_wo, n_dot_t, eta, result);
    }
}

void Sample::reflect(float3 const& wo, float3 const& n, float n_dot_wo,
                     bxdf::Sample& result) noexcept {
    result.reflection = float3(1.f);
    result.wi         = normalize(2.f * n_dot_wo * n - wo);
    result.pdf        = 1.f;
    result.type.clear(bxdf::Type::Specular_reflection);

    //    SOFT_ASSERT(testing::check(result, sample.wo_, layer));
}

void Sample::refract(float3 const& wo, float3 const& n, float3 const& color, float n_dot_wo,
                     float n_dot_t, float eta, bxdf::Sample& result) noexcept {
    result.reflection = color;
    result.wi         = normalize((eta * n_dot_wo - n_dot_t) * n - eta * wo);
    result.pdf        = 1.f;
    result.type.clear(bxdf::Type::Specular_transmission);

    //    SOFT_ASSERT(testing::check(result, sample.wo_, layer));
}

}  // namespace scene::material::glass
