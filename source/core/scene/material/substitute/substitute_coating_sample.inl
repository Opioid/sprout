#ifndef SU_CORE_SCENE_MATERIAL_SUBSTITUTE_COATING_SAMPLE_INL
#define SU_CORE_SCENE_MATERIAL_SUBSTITUTE_COATING_SAMPLE_INL

#include "base/math/math.hpp"
#include "base/math/vector3.inl"
#include "scene/material/material_sample.inl"
#include "substitute_base_sample.inl"
#include "substitute_coating_sample.hpp"

namespace scene::material::substitute {

template <typename Coating, typename Diffuse>
float3 Sample_coating<Coating, Diffuse>::radiance() const {
    float const n_dot_wo = coating_.clamp_abs_n_dot(wo_);

    return coating_.attenuation(n_dot_wo) * base_.emission_;
}

template <typename Coating, typename Diffuse>
bxdf::Result Sample_coating<Coating, Diffuse>::evaluate_f(float3 const& wi,
                                                          bool /*include_back*/) const {
    if (!same_hemisphere(wo_)) {
        return {float3(0.f), 0.f};
    }

    return evaluate<true>(wi);
}

template <typename Coating, typename Diffuse>
bxdf::Result Sample_coating<Coating, Diffuse>::evaluate_b(float3 const& wi,
                                                          bool /*include_back*/) const {
    if (!same_hemisphere(wo_)) {
        return {float3(0.f), 0.f};
    }

    return evaluate<false>(wi);
}

template <typename Coating, typename Diffuse>
void Sample_coating<Coating, Diffuse>::sample(Sampler& sampler, bxdf::Sample& result) const {
    if (!same_hemisphere(wo_)) {
        result.pdf = 0.f;
        return;
    }

    if (float const p = sampler.generate_sample_1D(); p < 0.5f) {
        coating_sample_and_base(sampler, result);
    } else {
        if (1.f == base_.metallic_) {
            pure_gloss_sample_and_coating(sampler, result);
        } else {
            if (p < 0.75f) {
                diffuse_sample_and_coating(sampler, result);
            } else {
                gloss_sample_and_coating(sampler, result);
            }
        }
    }

    result.wavelength = 0.f;
}

template <typename Coating, typename Diffuse>
template <bool Forward>
bxdf::Result Sample_coating<Coating, Diffuse>::evaluate(float3 const& wi) const {
    float3 const h = normalize(wo_ + wi);

    float const wo_dot_h = clamp_dot(wo_, h);

    auto const coating = coating_.evaluate_f(wi, wo_, h, wo_dot_h, base_.avoid_caustics_);

    float const metallic = base_.metallic_;

    auto const base = 1.f == metallic
                          ? base_.template pure_gloss_evaluate<Forward>(wi, wo_, h, wo_dot_h,
                                                                        layer_)
                          : base_.template base_evaluate<Forward>(wi, wo_, h, wo_dot_h, layer_);

    float const pdf = 0.5f * (coating.pdf + base.pdf());
    return {coating.reflection + coating.attenuation * base.reflection, pdf};
}

template <typename Coating, typename Diffuse>
void Sample_coating<Coating, Diffuse>::coating_sample_and_base(Sampler&      sampler,
                                                               bxdf::Sample& result) const {
    float3 coating_attenuation;
    coating_.sample(wo_, sampler, coating_attenuation, result);

    auto const base = 1.f == base_.metallic_
                          ? base_.template pure_gloss_evaluate<true>(result.wi, wo_, result.h,
                                                                     result.h_dot_wi, layer_)
                          : base_.template base_evaluate<true>(result.wi, wo_, result.h,
                                                               result.h_dot_wi, layer_);

    result.reflection = result.reflection + coating_attenuation * base.reflection;
    result.pdf        = 0.5f * (result.pdf + base.pdf());
}

template <typename Coating, typename Diffuse>
void Sample_coating<Coating, Diffuse>::diffuse_sample_and_coating(Sampler&      sampler,
                                                                  bxdf::Sample& result) const {
    base_.diffuse_sample(wo_, layer_, sampler, base_.avoid_caustics_, result);

    auto const coating = coating_.evaluate_f(result.wi, wo_, result.h, result.h_dot_wi,
                                             base_.avoid_caustics_);

    result.reflection = coating.attenuation * result.reflection + coating.reflection;
    result.pdf        = 0.5f * (result.pdf + coating.pdf);
}

template <typename Coating, typename Diffuse>
void Sample_coating<Coating, Diffuse>::gloss_sample_and_coating(Sampler&      sampler,
                                                                bxdf::Sample& result) const {
    base_.gloss_sample(wo_, layer_, sampler, result);

    auto const coating = coating_.evaluate_f(result.wi, wo_, result.h, result.h_dot_wi,
                                             base_.avoid_caustics_);

    result.reflection = coating.attenuation * result.reflection + coating.reflection;
    result.pdf        = 0.5f * (result.pdf + coating.pdf);
}

template <typename Coating, typename Diffuse>
void Sample_coating<Coating, Diffuse>::pure_gloss_sample_and_coating(Sampler&      sampler,
                                                                     bxdf::Sample& result) const {
    base_.pure_gloss_sample(wo_, layer_, sampler, result);

    auto const coating = coating_.evaluate_f(result.wi, wo_, result.h, result.h_dot_wi,
                                             base_.avoid_caustics_);

    result.reflection = coating.attenuation * result.reflection + coating.reflection;
    result.pdf        = 0.5f * (result.pdf + coating.pdf);
}

}  // namespace scene::material::substitute

#endif
