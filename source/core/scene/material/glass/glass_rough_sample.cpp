#include "glass_rough_sample.hpp"
#include "base/math/math.hpp"
#include "base/math/vector3.inl"
#include "sampler/sampler.hpp"
#include "scene/material/bxdf.hpp"
#include "scene/material/collision_coefficients.inl"
#include "scene/material/fresnel/fresnel.inl"
#include "scene/material/ggx/ggx.inl"
#include "scene/material/material_sample.inl"

#include "base/debug/assert.hpp"
#include "scene/material/material_test.hpp"

namespace scene::material::glass {

bxdf::Result Sample_rough::evaluate_f(float3 const& wi) const {
    return evaluate<true>(wi);
}

bxdf::Result Sample_rough::evaluate_b(float3 const& wi) const {
    return evaluate<false>(wi);
}

void Sample_rough::sample(Sampler& sampler, RNG& rng, bxdf::Sample& result) const {
    if (ior_.eta_i == ior_.eta_t) {
        result.reflection = albedo_;
        result.wi         = -wo_;
        result.pdf        = 1.f;
        result.wavelength = 0.f;
        result.type.clear(bxdf::Type::Straight_transmission);
        return;
    }

    bool const same_side = same_hemisphere(wo_);

    Layer const layer = layer_.swapped(same_side);

    IoR const ior = ior_.swapped(same_side);

    float2 const xi = sampler.generate_sample_2D(rng);

    float        n_dot_h;
    float3 const h = ggx::Isotropic::sample(wo_, layer, alpha_, xi, n_dot_h);

    float const n_dot_wo = layer.clamp_abs_n_dot(wo_);

    float const wo_dot_h = clamp_dot(wo_, h);

    float const eta = ior.eta_i / ior.eta_t;

    float const sint2 = (eta * eta) * (1.f - wo_dot_h * wo_dot_h);

    float f;
    float wi_dot_h;

    if (sint2 >= 1.f) {
        f        = 1.f;
        wi_dot_h = 0.f;
    } else {
        wi_dot_h = std::sqrt(1.f - sint2);

        float const cos_x = ior.eta_i > ior.eta_t ? wi_dot_h : wo_dot_h;

        f = fresnel::schlick(cos_x, f0_);
    }

    if (sampler.generate_sample_1D(rng) <= f) {
        float const n_dot_wi = ggx::Isotropic::reflect(wo_, h, n_dot_wo, n_dot_h, wi_dot_h,
                                                       wo_dot_h, layer, alpha_, result);

        result.reflection *= f * n_dot_wi;
        result.pdf *= f;
        if (!same_side) {
            result.type.set(bxdf::Type::Caustic);
        }
    } else {
        float const r_wo_dot_h = same_side ? -wo_dot_h : wo_dot_h;

        float const n_dot_wi = ggx::Isotropic::refract(wo_, h, n_dot_wo, n_dot_h, -wi_dot_h,
                                                       r_wo_dot_h, layer, alpha_, ior, result);

        float const omf = 1.f - f;

        result.reflection *= omf * n_dot_wi * albedo_;
        result.pdf *= omf;
        result.type.set(bxdf::Type::Caustic);
    }

    result.reflection *= ggx::ilm_ep_dielectric(n_dot_wo, alpha_, ior_.eta_t);

    result.wavelength = 0.f;
}

void Sample_rough::set(float ior, float ior_outside) {
    properties_.set(Property::Can_evaluate, ior != ior_outside);

    f0_ = fresnel::schlick_f0(ior, ior_outside);

    ior_.eta_t = ior;
    ior_.eta_i = ior_outside;
}

template <bool Forward>
bxdf::Result Sample_rough::evaluate(float3 const& wi) const {
    if (ior_.eta_i == ior_.eta_t) {
        return {float3(0.f), 0.f};
    }

    if (!same_hemisphere(wo_)) {
        if (avoid_caustics()) {
            return {float3(0.f), 0.f};
        }

        IoR const ior = ior_.swapped();

        float3 const h = -normalize(ior.eta_t * wi + ior.eta_i * wo_);

        float const wi_dot_h = dot(wi, h);
        if (wi_dot_h <= 0.f) {
            return {float3(0.f), 0.f};
        }

        float const eta = ior.eta_i / ior.eta_t;

        float const wo_dot_h = dot(wo_, h);

        float const sint2 = (eta * eta) * (1.f - wo_dot_h * wo_dot_h);

        if (sint2 >= 1.f) {
            return {float3(0.f), 0.f};
        }

        float const n_dot_wi = layer_.clamp_n_dot(wi);
        float const n_dot_wo = layer_.clamp_abs_n_dot(wo_);
        float const n_dot_h  = saturate(dot(layer_.n_, h));

        fresnel::Schlick1 const schlick(f0_);

        auto ggx = ggx::Isotropic::refraction(n_dot_wi, n_dot_wo, wi_dot_h, wo_dot_h, n_dot_h,
                                              alpha_, ior, schlick);

        ggx.reflection *= ggx::ilm_ep_dielectric(n_dot_wo, alpha_, ior_.eta_t);

        if constexpr (Forward) {
            return {std::min(n_dot_wi, n_dot_wo) * albedo_ * ggx.reflection, ggx.pdf()};
        } else {
            return {albedo_ * ggx.reflection, ggx.pdf()};
        }
    } else {
        float const n_dot_wi = layer_.clamp_n_dot(wi);
        float const n_dot_wo = layer_.clamp_abs_n_dot(wo_);

        float3 const h = normalize(wo_ + wi);

        float const wo_dot_h = clamp_dot(wo_, h);
        float const n_dot_h  = saturate(dot(layer_.n_, h));

        fresnel::Schlick const schlick(f0_);

        float3 fresnel;

        auto ggx = ggx::Isotropic::reflection(n_dot_wi, n_dot_wo, wo_dot_h, n_dot_h, alpha_,
                                              schlick, fresnel);

        ggx.reflection *= ggx::ilm_ep_dielectric(n_dot_wo, alpha_, ior_.eta_t);

        if constexpr (Forward) {
            return {n_dot_wi * ggx.reflection, fresnel[0] * ggx.pdf()};
        } else {
            return {ggx.reflection, fresnel[0] * ggx.pdf()};
        }
    }
}

}  // namespace scene::material::glass
