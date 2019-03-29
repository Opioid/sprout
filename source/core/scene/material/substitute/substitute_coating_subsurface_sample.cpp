#include "substitute_coating_subsurface_sample.hpp"
#include "base/math/math.hpp"
#include "sampler/sampler.hpp"
#include "scene/material/bxdf.hpp"
#include "scene/material/fresnel/fresnel.inl"
#include "scene/material/material_sample.inl"
#include "substitute_base_sample.inl"

namespace scene::material::substitute {

using Clearcoat_no_lambert = Sample_coating<coating::Clearcoat_layer, disney::Isotropic_no_lambert>;

bxdf::Result Sample_coating_subsurface::evaluate_f(float3 const& wi, bool include_back) const
    noexcept {
    return evaluate<true>(wi, include_back);
}

bxdf::Result Sample_coating_subsurface::evaluate_b(float3 const& wi, bool include_back) const
    noexcept {
    return evaluate<false>(wi, include_back);
}

void Sample_coating_subsurface::sample(sampler::Sampler& sampler, bxdf::Sample& result) const
    noexcept {
    if (1.f == metallic_) {
        Clearcoat_no_lambert::sample(sampler, result);
        return;
    }

    if (ior_.eta_i == ior_.eta_t) {
        result.reflection = float3(1.f);
        result.wi         = -wo_;
        result.pdf        = 1.f;
        result.wavelength = 0.f;
        result.type.clear(bxdf::Type::Specular_transmission);
        return;
    }

    bool const same_side = same_hemisphere(wo_);

    float const p = sampler.generate_sample_1D();

    if (same_side) {
        if (p < 0.5f) {
            refract(sampler, result);
        } else {
            if (p < 0.75f) {
                float3 coating_attenuation;
                coating_.sample(wo_, sampler, coating_attenuation, result);

                auto const base = 1.f == metallic_
                                      ? pure_gloss_evaluate<true>(result.wi, wo_, result.h,
                                                                  result.h_dot_wi, avoid_caustics_)
                                      : base_evaluate<true>(result.wi, wo_, result.h,
                                                            result.h_dot_wi, avoid_caustics_);

                result.reflection = result.reflection + coating_attenuation * base.reflection;
                result.pdf        = 0.5f * (result.pdf + base.pdf);
            } else {
                if (1.f == metallic_) {
                    pure_gloss_sample_and_coating(sampler, result);
                } else {
                    if (p < 0.875f) {
                        diffuse_sample_and_coating(sampler, result);
                    } else {
                        gloss_sample_and_coating(sampler, result);
                    }
                }
            }
        }

        result.pdf *= 0.5f;

    } else {
        Layer const layer = layer_.swapped();

        IoR const ior = ior_.swapped();

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

            f = fresnel::schlick(cos_x, f0_[0]);
        }

        if (p < f) {
            float const n_dot_wi = ggx::Isotropic::reflect(wo_, h, n_dot_wo, n_dot_h, wi_dot_h,
                                                           wo_dot_h, layer, alpha_, result);

            result.reflection *= n_dot_wi;
        } else {
            float const r_wo_dot_h = same_side ? -wo_dot_h : wo_dot_h;

            float const n_dot_wi = ggx::Isotropic::refract(wo_, h, n_dot_wo, n_dot_h, -wi_dot_h,
                                                           r_wo_dot_h, layer, alpha_, ior, result);

            result.reflection *= n_dot_wi;
        }

        result.type.set(bxdf::Type::Caustic);
    }

    result.wavelength = 0.f;
}

bool Sample_coating_subsurface::do_evaluate_back(bool previously, bool same_side) const noexcept {
    return previously || same_side;
}

void Sample_coating_subsurface::set_volumetric(float anisotropy, float ior,
                                               float ior_outside) noexcept {
    anisotropy_ = anisotropy;

    ior_.eta_t = ior;
    ior_.eta_i = ior_outside;
}

template <bool Forward>
bxdf::Result Sample_coating_subsurface::evaluate(float3 const& wi, bool include_back) const
    noexcept {
    if (ior_.eta_i == ior_.eta_t) {
        return {float3(0.f), 0.f};
    }

    if (!same_hemisphere(wo_)) {
        if (!include_back) {
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

        fresnel::Schlick1 const schlick(f0_[0]);

        auto const ggx = ggx::Isotropic::refraction(n_dot_wi, n_dot_wo, wi_dot_h, wo_dot_h, n_dot_h,
                                                    alpha_, ior, schlick);

        if (Forward) {
            return {std::min(n_dot_wi, n_dot_wo) * ggx.reflection, ggx.pdf};
        } else {
            return ggx;
        }
    }

    auto result = Clearcoat_no_lambert::evaluate<Forward>(wi);

    if (1.f != metallic_) {
        result.pdf *= 0.5f;
    }

    return result;
}

void Sample_coating_subsurface::refract(sampler::Sampler& sampler, bxdf::Sample& result) const
    noexcept {
    if (ior_.eta_i == ior_.eta_t) {
        result.reflection = float3(1.f);
        result.wi         = -wo_;
        result.pdf        = 1.f;
        result.wavelength = 0.f;
        result.type.clear(bxdf::Type::Specular_transmission);
        return;
    }

    float const n_dot_wo = layer_.clamp_abs_n_dot(wo_);

    fresnel::Schlick1 const schlick(f0_[0]);

    float const n_dot_wi = ggx::Isotropic::refract(wo_, n_dot_wo, layer_, alpha_, ior_, schlick,
                                                   sampler, result);

    result.reflection *= n_dot_wi;
    result.type.set(bxdf::Type::Caustic);
}

bxdf::Result Sample_coating_subsurface_volumetric::evaluate_f(float3 const& wi,
                                                              bool /*include_back*/) const
    noexcept {
    bxdf::Result result = volumetric::Sample::evaluate_f(wi, true);

    // Fresnel is only part of evaluate() because it tries to compensate for the fact,
    // that direct light calculations for SSS in the integrators are ignoring one surface.
    float3 const h = normalize(wo_ + wi);

    float const wo_dot_h = clamp_abs_dot(wo_, h);

    float const f = 1.f - fresnel::schlick(wo_dot_h, f0_);

    result.reflection *= f;

    return result;
}

bxdf::Result Sample_coating_subsurface_volumetric::evaluate_b(float3 const& wi,
                                                              bool /*include_back*/) const
    noexcept {
    bxdf::Result result = volumetric::Sample::evaluate_b(wi, true);

    // Fresnel is only part of evaluate() because it tries to compensate for the fact,
    // that direct light calculations for SSS in the integrators are ignoring one surface.
    float3 const h = normalize(wo_ + wi);

    float const wo_dot_h = clamp_abs_dot(wo_, h);

    float const f = 1.f - fresnel::schlick(wo_dot_h, f0_);

    result.reflection *= f;

    return result;
}

bool Sample_coating_subsurface_volumetric::do_evaluate_back(bool /*previously*/,
                                                            bool /*same_side*/) const noexcept {
    return false;
}

void Sample_coating_subsurface_volumetric::set(float anisotropy, float f0) noexcept {
    volumetric::Sample::set(anisotropy);

    f0_ = f0;
}

}  // namespace scene::material::substitute
