#include "substitute_subsurface_sample.hpp"
#include "base/math/math.hpp"
#include "sampler/sampler.hpp"
#include "scene/material/bxdf.hpp"
#include "scene/material/material_sample.inl"
#include "substitute_base_sample.inl"

namespace scene::material::substitute {

bxdf::Result Sample_subsurface::evaluate(float3 const& wi) const noexcept {
    if (!same_hemisphere(wo_)) {
        return {float3::identity(), 0.f};
    }

    float3 const h = math::normalize(wo_ + wi);

    float const wo_dot_h = clamp_dot(wo_, h);

    auto result = base_evaluate(wi, wo_, h, wo_dot_h, avoid_caustics_);
    result.pdf *= 0.5f;
    return result;
}

void Sample_subsurface::sample(sampler::Sampler& sampler, bxdf::Sample& result) const noexcept {
    bool const same_side = same_hemisphere(wo_);

    float const p = sampler.generate_sample_1D();

    if (same_side) {
        if (p < 0.5f) {
            refract(same_side, layer_, sampler, result);
        } else {
            if (p < 0.75f) {
                diffuse_sample(wo_, sampler, avoid_caustics_, result);
            } else {
                gloss_sample(wo_, sampler, result);
            }
        }
        result.pdf *= 0.5f;
    } else {
        Layer layer = layer_;
        layer.n_    = -layer_.n_;

        IoR ior = ior_.swapped();

        float        n_dot_h;
        float3 const h = ggx::Isotropic::sample(wo_, layer, alpha_, sampler, n_dot_h);

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

            fresnel::Schlick1 const schlick(f0_[0]);
            f = schlick(cos_x);
        }

        if (p < f) {
            float const n_dot_wi = ggx::Isotropic::reflect(wo_, h, n_dot_wo, n_dot_h, wi_dot_h,
                                                           wo_dot_h, layer, alpha_, result);
            result.reflection *= n_dot_wi;
        } else {
            float const n_dot_wi = ggx::Isotropic::refract(wo_, h, n_dot_wo, n_dot_h, wi_dot_h,
                                                           wo_dot_h, layer, alpha_, ior, result);
            result.reflection *= n_dot_wi;
        }
    }

    result.wavelength = 0.f;
}

void Sample_subsurface::set_volumetric(float anisotropy, float ior, float ior_outside) noexcept {
    anisotropy_ = anisotropy;

    ior_.eta_t = ior;
    ior_.eta_i = ior_outside;
}

void Sample_subsurface::refract(bool same_side, Layer const& layer, sampler::Sampler& sampler,
                                bxdf::Sample& result) const noexcept {
    IoR tmp_ior = ior_.swapped(same_side);

    float const n_dot_wo = layer.clamp_abs_n_dot(wo_);

    fresnel::Schlick1 const schlick(f0_[0]);

    float const n_dot_wi = ggx::Isotropic::refract(wo_, n_dot_wo, layer, alpha_, tmp_ior, schlick,
                                                   sampler, result);

    result.reflection *= n_dot_wi;
}

bxdf::Result Sample_subsurface_volumetric::evaluate(float3 const& wi) const noexcept {
    bxdf::Result result = volumetric::Sample::evaluate(wi);

    // Fresnel is only part of evaluate() because it tries to compensate for the fact,
    // that direct light calculations for SSS in the integrators are ignoring one surface.
    float3 const h = math::normalize(wo_ + wi);

    float const wo_dot_h = clamp_abs_dot(wo_, h);

    float const f = 1.f - fresnel::schlick(wo_dot_h, f0_);

    result.reflection *= f;

    return result;
}

void Sample_subsurface_volumetric::set(float anisotropy, float f0) noexcept {
    volumetric::Sample::set(anisotropy);

    f0_ = f0;
}

}  // namespace scene::material::substitute
