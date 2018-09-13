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

const material::Sample::Layer& Sample::base_layer() const noexcept {
    return layer_;
}

bxdf::Result Sample::evaluate(float3 const& /*wi*/) const noexcept {
    return {float3::identity(), 0.f};
}

void Sample::sample(sampler::Sampler& sampler, bxdf::Sample& result) const noexcept {
    float3 n = layer_.n_;

    float eta_i = ior_outside_;
    float eta_t = ior_;

    if (!same_hemisphere(wo_)) {
        n = -n;

        std::swap(eta_i, eta_t);
    }

    float const n_dot_wo = std::min(std::abs(math::dot(n, wo_)), 1.f);
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

    if (sampler.generate_sample_1D() < f) {
        BSDF::reflect(wo_, n, n_dot_wo, result);
    } else {
        BSDF::refract(wo_, n, color_, n_dot_wo, n_dot_t, eta, result);
    }

    result.wavelength = 0.f;
}

void Sample::set(float3 const& refraction_color, float ior, float ior_outside) noexcept {
    color_       = refraction_color;
    ior_         = ior;
    ior_outside_ = ior_outside;
}

float Sample::BSDF::reflect(float3 const& wo, float3 const& n, float n_dot_wo,
                            bxdf::Sample& result) noexcept {
    result.reflection = float3(1.f);
    result.wi         = math::normalize(2.f * n_dot_wo * n - wo);
    result.pdf        = 1.f;
    result.type.clear(bxdf::Type::Specular_reflection);

    //    SOFT_ASSERT(testing::check(result, sample.wo_, layer));

    return 1.f;
}

float Sample::BSDF::refract(float3 const& wo, float3 const& n, float3 const& color, float n_dot_wo,
                            float n_dot_t, float eta, bxdf::Sample& result) noexcept {
    result.reflection = color;
    result.wi         = math::normalize((eta * n_dot_wo - n_dot_t) * n - eta * wo);
    result.pdf        = 1.f;
    result.type.clear(bxdf::Type::Specular_transmission);

    //    SOFT_ASSERT(testing::check(result, sample.wo_, layer));

    return 1.f;
}

}  // namespace scene::material::glass
