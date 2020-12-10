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
        float       n_dot_h;
        float const cf = coating_.sample(wo_, sampler, rng, n_dot_h, result);

        if (p <= cf) {
            coating_reflect(cf, n_dot_h, result);
        } else {
            Layer const layer = layer_.swapped(same_side);

            IoR const ior = ior_.swapped(same_side);

            float2 const xi = sampler.sample_2D(rng);

            float        n_dot_h;
            float3 const h = ggx::Isotropic::sample(wo_, alpha_, xi, layer, n_dot_h);

            float const n_dot_wo = layer.clamp_abs_n_dot(wo_);
            float const wo_dot_h = clamp_dot(wo_, h);

            float const eta   = ior.eta_i / ior.eta_t;
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

            if (p <= f) {
                float const n_dot_wi = ggx::Isotropic::reflect(wo_, h, n_dot_wo, n_dot_h, wi_dot_h,
                                                               wo_dot_h, alpha_, layer_, result);

                auto const d = Diffuse::reflection(result.h_dot_wi, n_dot_wi, n_dot_wo, alpha_,
                                                   albedo_);

                float3 const refl = n_dot_wi * (f * result.reflection + d.reflection);
                result.reflection = refl * ggx::ilm_ep_conductor(base_.f0_, n_dot_wo, alpha_);
                result.pdf        = f * result.pdf;
            } else {
                float const r_wo_dot_h = -wo_dot_h;

                float const n_dot_wi = ggx::Isotropic::refract(
                    wo_, h, n_dot_wo, n_dot_h, -wi_dot_h, r_wo_dot_h, alpha_, ior, layer_, result);

                float const omf = 1.f - f;

                float const coating_n_dot_wo = coating_.clamp_abs_n_dot(wo_);

                // Approximating the full coating attenuation at entrance, for the benefit of SSS,
                // which will ignore the border later.
                // This will probably cause problems for shapes intersecting such materials.
                float3 const attenuation = coating_.attenuation(0.5f, coating_n_dot_wo);

                result.reflection *= omf * n_dot_wi * attenuation;
                result.pdf *= omf;
            }

            result.pdf *= 1.f - cf;
        }
    } else {
        Layer const layer = layer_.swapped(same_side);

        IoR const ior = ior_.swapped(same_side);

        float2 const xi = sampler.sample_2D(rng);

        float        n_dot_h;
        float3 const h = ggx::Isotropic::sample(wo_, alpha_, xi, layer, n_dot_h);

        float const n_dot_wo = layer.clamp_abs_n_dot(wo_);
        float const wo_dot_h = clamp_dot(wo_, h);

        float const eta   = ior.eta_i / ior.eta_t;
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
                                                           wo_dot_h, alpha_, layer, result);

            result.reflection *= f * n_dot_wi;
            result.pdf *= f;
        } else {
            float const r_wo_dot_h = same_side ? -wo_dot_h : wo_dot_h;

            float const n_dot_wi = ggx::Isotropic::refract(wo_, h, n_dot_wo, n_dot_h, -wi_dot_h,
                                                           r_wo_dot_h, alpha_, ior, layer, result);

            float const coating_n_dot_wo = coating_.clamp_abs_n_dot(wo_);

            float3 const attenuation = coating_.attenuation(coating_n_dot_wo);

            float const omf = 1.f - f;

            result.reflection *= omf * n_dot_wi * attenuation;
            result.pdf *= omf;
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

        auto ggx = ggx::Isotropic::refraction(n_dot_wi, n_dot_wo, wi_dot_h, wo_dot_h, n_dot_h,
                                              alpha_, ior, schlick);

        ggx.reflection *= ggx::ilm_ep_dielectric(n_dot_wo, alpha_, ior_.eta_t);

        if (Forward) {
            return {std::min(n_dot_wi, n_dot_wo) * ggx.reflection, ggx.pdf()};
        }

        return ggx;
    }

    float3 const h = normalize(wo_ + wi);

    float const wo_dot_h = clamp_dot(wo_, h);

    auto const coating = coating_.evaluate_f(wi, wo_, h, wo_dot_h, avoid_caustics());

    // TODO we need coating here as well
    if (1.f == base_.metallic_) {
        return base_.pure_gloss_evaluate<Forward>(wi, wo_, h, wo_dot_h, *this);
    }

    Layer const& layer = layer_;

    float const alpha = alpha_;

    float const n_dot_wi = layer.clamp_n_dot(wi);
    float const n_dot_wo = layer.clamp_abs_n_dot(wo_);

    auto const d = Diffuse::reflection(wo_dot_h, n_dot_wi, n_dot_wo, alpha, albedo_);

    if (avoid_caustics() && alpha <= ggx::Min_alpha) {
        if constexpr (Forward) {
            return {n_dot_wi * d.reflection, d.pdf()};
        } else {
            return d;
        }
    }

    float const n_dot_h = saturate(layer.n_dot(h));

    fresnel::Schlick const schlick(base_.f0_);

    float3 fresnel_result;
    auto   ggx = ggx::Isotropic::reflection(n_dot_wi, n_dot_wo, wo_dot_h, n_dot_h, alpha, schlick,
                                          fresnel_result);

    ggx.reflection *= ggx::ilm_ep_conductor(base_.f0_, n_dot_wo, alpha);

    float const base_pdf = fresnel_result[0] * ggx.pdf();

    float const pdf = coating.f * coating.pdf + (1.f - coating.f) * base_pdf;
    return {coating.reflection + n_dot_wi * coating.attenuation * ggx.reflection, pdf};
}

}  // namespace scene::material::substitute
