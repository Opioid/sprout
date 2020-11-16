#include "substitute_coating_subsurface_sample.hpp"
#include "base/math/math.hpp"
#include "sampler/sampler.hpp"
#include "scene/material/bxdf.hpp"
#include "scene/material/fresnel/fresnel.inl"
#include "scene/material/material_sample.inl"
#include "substitute_coating_sample.inl"

namespace scene::material::substitute {

using Clearcoat_no_lambert = Sample_coating<coating::Clearcoat_layer, disney::Isotropic_no_lambert>;

bxdf::Result Sample_coating_subsurface::evaluate_f(float3_p wi) const {
    return evaluate<true>(wi);
}

bxdf::Result Sample_coating_subsurface::evaluate_b(float3_p wi) const {
    return evaluate<false>(wi);
}

void Sample_coating_subsurface::sample(Sampler& sampler, RNG& rng, bxdf::Sample& result) const {
    if (1.f == base_.metallic_) {
        Clearcoat_no_lambert::sample(sampler, rng, result);
        return;
    }

    if (ior_.eta_i == ior_.eta_t) {
        result.reflection = float3(1.f);
        result.wi         = -wo_;
        result.pdf        = 1.f;
        result.wavelength = 0.f;
        result.type.clear(bxdf::Type::Straight_transmission);
        return;
    }

    bool const same_side = same_hemisphere(wo_);

    float const p = sampler.sample_1D(rng);

    if (same_side) {
        if (p < 0.5f) {
            refract(sampler, rng, result);
        } else {
            if (p < 0.75f) {
                coating_sample_and_base(sampler, rng, result);
            } else {
                if (1.f == base_.metallic_) {
                    pure_gloss_sample_and_coating(sampler, rng, result);
                } else {
                    if (p < 0.875f) {
                        diffuse_sample_and_coating(sampler, rng, result);
                    } else {
                        gloss_sample_and_coating(sampler, rng, result);
                    }
                }
            }
        }

        result.pdf *= 0.5f;
    } else {
        Layer const layer = layer_.swapped();

        IoR const ior = ior_.swapped();

        float2 const xi = sampler.sample_2D(rng);

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

            f = fresnel::schlick(cos_x, base_.f0_[0]);
        }

        if (p < f) {
            float const n_dot_wi = ggx::Isotropic::reflect(wo_, h, n_dot_wo, n_dot_h, wi_dot_h,
                                                           wo_dot_h, layer, alpha_, result);

            result.reflection *= n_dot_wi;
        } else {
            float const r_wo_dot_h = same_side ? -wo_dot_h : wo_dot_h;

            float const n_dot_wi = ggx::Isotropic::refract(wo_, h, n_dot_wo, n_dot_h, -wi_dot_h,
                                                           r_wo_dot_h, layer, alpha_, ior, result);

            float const coating_n_dot_wo = coating_.clamp_abs_n_dot(wo_);

            float3 const attenuation = coating_.attenuation(coating_n_dot_wo);

            result.reflection *= n_dot_wi * attenuation;
        }
    }

    result.wavelength = 0.f;
}

void Sample_coating_subsurface::set_volumetric(float ior, float ior_outside) {
    properties_.set(Property::Can_evaluate, ior != ior_outside);

    ior_.eta_t = ior;
    ior_.eta_i = ior_outside;
}

template <bool Forward>
bxdf::Result Sample_coating_subsurface::evaluate(float3_p wi) const {
    if (ior_.eta_i == ior_.eta_t) {
        return {float3(0.f), 0.f};
    }

    if (!same_hemisphere(wo_)) {
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

        fresnel::Schlick1 const schlick(base_.f0_[0]);

        auto const ggx = ggx::Isotropic::refraction(n_dot_wi, n_dot_wo, wi_dot_h, wo_dot_h, n_dot_h,
                                                    alpha_, ior, schlick);

        if (Forward) {
            return {std::min(n_dot_wi, n_dot_wo) * ggx.reflection, ggx.pdf()};
        }

        return {ggx.reflection, ggx.pdf()};
    }

    auto result = Clearcoat_no_lambert::evaluate<Forward>(wi);

    if (1.f != base_.metallic_) {
        result.pdf() *= 0.5f;
    }

    return result;
}

void Sample_coating_subsurface::refract(Sampler& sampler, RNG& rng, bxdf::Sample& result) const {
    float const n_dot_wo = layer_.clamp_abs_n_dot(wo_);

    fresnel::Schlick1 const schlick(base_.f0_[0]);

    float2 const xi = sampler.sample_2D(rng);

    float const n_dot_wi = ggx::Isotropic::refract(wo_, n_dot_wo, layer_, alpha_, ior_, schlick, xi,
                                                   result);

    float const coating_n_dot_wo = coating_.clamp_abs_n_dot(wo_);

    // Approximating the full coating attenuation at entrance, for the benefit of SSS,
    // which will ignore the border later.
    // This will probably cause problems for shapes intersecting such materials.
    float3 const attenuation = coating_.attenuation(0.5f, coating_n_dot_wo);

    result.reflection *= n_dot_wi * attenuation;
}

}  // namespace scene::material::substitute
