#include "substitute_subsurface_sample.hpp"
#include "base/math/math.hpp"
#include "sampler/sampler.hpp"
#include "scene/material/bxdf.hpp"
#include "scene/material/fresnel/fresnel.inl"
#include "scene/material/material_sample.inl"
#include "substitute_base_sample.inl"

namespace scene::material::substitute {

bxdf::Result Sample_subsurface::evaluate(float3_p wi) const {
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

        float const wo_dot_h = dot(wo_, h);

        float const eta   = ior.eta_i / ior.eta_t;
        float const sint2 = (eta * eta) * (1.f - wo_dot_h * wo_dot_h);

        if (sint2 >= 1.f) {
            return {float3(0.f), 0.f};
        }

        float const n_dot_wi = layer_.clamp_n_dot(wi);
        float const n_dot_wo = layer_.clamp_abs_n_dot(wo_);
        float const n_dot_h  = saturate(dot(layer_.n_, h));

        fresnel::Schlick1 const schlick(base_.f0_[0]);

        auto ggx = ggx::Iso::refraction(n_dot_wi, n_dot_wo, wi_dot_h, wo_dot_h, n_dot_h, alpha_,
                                        ior, schlick);

        ggx.reflection *= ggx::ilm_ep_dielectric(n_dot_wo, alpha_, ior_.eta_t);

        return {std::min(n_dot_wi, n_dot_wo) * ggx.reflection, ggx.pdf()};
    }

    float3 const h = normalize(wo_ + wi);

    float const wo_dot_h = clamp_dot(wo_, h);

    if (1.f == base_.metallic_) {
        return base_.pure_gloss_evaluate(wi, wo_, h, wo_dot_h, *this);
    }

    Layer const& layer = layer_;

    float const alpha = alpha_;

    float const n_dot_wi = layer.clamp_n_dot(wi);
    float const n_dot_wo = layer.clamp_abs_n_dot(wo_);

    auto const d = Diffuse::reflection(wo_dot_h, n_dot_wi, n_dot_wo, alpha, albedo_);

    if (avoid_caustics() && alpha <= ggx::Min_alpha) {
        return {n_dot_wi * d.reflection, d.pdf()};
    }

    float const n_dot_h = saturate(layer.n_dot(h));

    fresnel::Schlick const schlick(base_.f0_);

    float3 fresnel_result;
    auto   ggx = ggx::Iso::reflection(n_dot_wi, n_dot_wo, wo_dot_h, n_dot_h, alpha, schlick,
                                    fresnel_result);

    ggx.reflection *= ggx::ilm_ep_conductor(base_.f0_, n_dot_wo, alpha, base_.metallic_);

    float const pdf = fresnel_result[0] * ggx.pdf();

    return {n_dot_wi * (d.reflection + ggx.reflection), pdf};
}

void Sample_subsurface::sample(Sampler& sampler, RNG& rng, bxdf::Sample& result) const {
    if (1.f == base_.metallic_) {
        base_.pure_gloss_sample(wo_, *this, sampler, rng, result);
        result.wavelength = 0.f;
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

    Layer const layer = layer_.swapped(same_side);

    IoR const ior = ior_.swapped(same_side);

    float2 const xi = sampler.sample_2D(rng);

    float        n_dot_h;
    float3 const h = ggx::Iso::sample(wo_, alpha_, xi, layer, n_dot_h);

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

    if (float const p = sampler.sample_1D(rng); same_side) {
        if (p <= f) {
            float const n_dot_wi = ggx::Iso::reflect(wo_, h, n_dot_wo, n_dot_h, wi_dot_h, wo_dot_h,
                                                     alpha_, layer, result);

            auto const d = Diffuse::reflection(result.h_dot_wi, n_dot_wi, n_dot_wo, alpha_,
                                               albedo_);

            float3 const reflection = n_dot_wi * (f * result.reflection + d.reflection);

            result.reflection = reflection * ggx::ilm_ep_conductor(base_.f0_, n_dot_wo, alpha_, base_.metallic_);
            result.pdf *= f;
        } else {
            float const r_wo_dot_h = -wo_dot_h;

            float const n_dot_wi = ggx::Iso::refract(wo_, h, n_dot_wo, n_dot_h, -wi_dot_h,
                                                     r_wo_dot_h, alpha_, ior, layer, result);

            float const omf = 1.f - f;

            result.reflection *= omf * n_dot_wi;
            result.pdf *= omf;
        }
    } else {
        if (p <= f) {
            float const n_dot_wi = ggx::Iso::reflect(wo_, h, n_dot_wo, n_dot_h, wi_dot_h, wo_dot_h,
                                                     alpha_, layer, result);

            result.reflection *= f * n_dot_wi;
            result.pdf *= f;
            //  result.type.set(bxdf::Type::Caustic);
        } else {
            float const r_wo_dot_h = wo_dot_h;

            float const n_dot_wi = ggx::Iso::refract(wo_, h, n_dot_wo, n_dot_h, -wi_dot_h,
                                                     r_wo_dot_h, alpha_, ior, layer, result);

            float const omf = 1.f - f;

            result.reflection *= omf * n_dot_wi;
            result.pdf *= omf;
            //  result.type.set(bxdf::Type::Caustic);
        }

        result.reflection *= ggx::ilm_ep_dielectric(n_dot_wo, alpha_, ior_.eta_t);
    }

    result.wavelength = 0.f;
}

void Sample_subsurface::set_volumetric(float ior, float ior_outside) {
    properties_.set(Property::Can_evaluate, ior != ior_outside);

    ior_.eta_t = ior;
    ior_.eta_i = ior_outside;
}

}  // namespace scene::material::substitute
