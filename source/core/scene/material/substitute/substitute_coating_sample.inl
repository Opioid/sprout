#ifndef SU_CORE_SCENE_MATERIAL_SUBSTITUTE_COATING_SAMPLE_INL
#define SU_CORE_SCENE_MATERIAL_SUBSTITUTE_COATING_SAMPLE_INL

#include "base/math/math.hpp"
#include "base/math/vector3.inl"
#include "scene/material/material_sample.inl"
#include "substitute_base_sample.inl"
#include "substitute_coating_sample.hpp"

namespace scene::material::substitute {

template <typename Coating, typename Diffuse>
float3 Sample_coating<Coating, Diffuse>::radiance() const noexcept {
    float const n_dot_wo = coating_.clamp_abs_n_dot(Sample_base<Diffuse>::wo_);

    return coating_.attenuation(n_dot_wo) * Sample_base<Diffuse>::emission_;
}

template <typename Coating, typename Diffuse>
bxdf::Result Sample_coating<Coating, Diffuse>::evaluate_f(float3 const& wi,
                                                          bool /*include_back*/) const noexcept {
    if (!Sample_base<Diffuse>::same_hemisphere(Sample_base<Diffuse>::wo_)) {
        return {float3(0.f), 0.f};
    }

    return evaluate<true>(wi);
}

template <typename Coating, typename Diffuse>
bxdf::Result Sample_coating<Coating, Diffuse>::evaluate_b(float3 const& wi,
                                                          bool /*include_back*/) const noexcept {
    if (!Sample_base<Diffuse>::same_hemisphere(Sample_base<Diffuse>::wo_)) {
        return {float3(0.f), 0.f};
    }

    return evaluate<false>(wi);
}

template <typename Coating, typename Diffuse>
void Sample_coating<Coating, Diffuse>::sample(sampler::Sampler& sampler, bxdf::Sample& result) const
    noexcept {
    if (!Sample_base<Diffuse>::same_hemisphere(Sample_base<Diffuse>::wo_)) {
        result.pdf = 0.f;
        return;
    }

    if (float const p = sampler.generate_sample_1D(); p < 0.5f) {
        float3 coating_attenuation;
        coating_.sample(Sample_base<Diffuse>::wo_, sampler, coating_attenuation, result);

        auto const base = 1.f == Sample_base<Diffuse>::metallic_
                              ? Sample_base<Diffuse>::pure_gloss_evaluate<true>(
                                    result.wi, Sample_base<Diffuse>::wo_, result.h, result.h_dot_wi,
                                    Sample_base<Diffuse>::avoid_caustics_)
                              : Sample_base<Diffuse>::base_evaluate<true>(
                                    result.wi, Sample_base<Diffuse>::wo_, result.h, result.h_dot_wi,
                                    Sample_base<Diffuse>::avoid_caustics_);

        result.reflection = result.reflection + coating_attenuation * base.reflection;
        result.pdf        = 0.5f * (result.pdf + base.pdf);
    } else {
        if (1.f == Sample_base<Diffuse>::metallic_) {
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
bxdf::Result Sample_coating<Coating, Diffuse>::evaluate(float3 const& wi) const noexcept {
    float3 const h = normalize(Sample_base<Diffuse>::wo_ + wi);

    float const wo_dot_h = clamp_dot(Sample_base<Diffuse>::wo_, h);

    auto const coating = coating_.evaluate_f(wi, Sample_base<Diffuse>::wo_, h, wo_dot_h,
                                             Sample_base<Diffuse>::avoid_caustics_);

    auto const base = 1.f == Sample_base<Diffuse>::metallic_
                          ? Sample_base<Diffuse>::pure_gloss_evaluate<Forward>(
                                wi, Sample_base<Diffuse>::wo_, h, wo_dot_h,
                                Sample_base<Diffuse>::avoid_caustics_)
                          : Sample_base<Diffuse>::base_evaluate<Forward>(
                                wi, Sample_base<Diffuse>::wo_, h, wo_dot_h,
                                Sample_base<Diffuse>::avoid_caustics_);

    float const pdf = 0.5f * (coating.pdf + base.pdf);
    return {coating.reflection + coating.attenuation * base.reflection, pdf};
}

template <typename Coating, typename Diffuse>
void Sample_coating<Coating, Diffuse>::diffuse_sample_and_coating(sampler::Sampler& sampler,
                                                                  bxdf::Sample&     result) const
    noexcept {
    diffuse_sample(Sample_base<Diffuse>::wo_, sampler, Sample_base<Diffuse>::avoid_caustics_,
                   result);

    auto const coating = coating_.evaluate_f(result.wi, Sample_base<Diffuse>::wo_, result.h,
                                             result.h_dot_wi,
                                             Sample_base<Diffuse>::avoid_caustics_);

    result.reflection = coating.attenuation * result.reflection + coating.reflection;
    result.pdf        = 0.5f * (result.pdf + coating.pdf);
}

template <typename Coating, typename Diffuse>
void Sample_coating<Coating, Diffuse>::gloss_sample_and_coating(sampler::Sampler& sampler,
                                                                bxdf::Sample&     result) const
    noexcept {
    gloss_sample(Sample_base<Diffuse>::wo_, sampler, result);

    auto const coating = coating_.evaluate_f(result.wi, Sample_base<Diffuse>::wo_, result.h,
                                             result.h_dot_wi,
                                             Sample_base<Diffuse>::avoid_caustics_);

    result.reflection = coating.attenuation * result.reflection + coating.reflection;
    result.pdf        = 0.5f * (result.pdf + coating.pdf);
}

template <typename Coating, typename Diffuse>
void Sample_coating<Coating, Diffuse>::pure_gloss_sample_and_coating(sampler::Sampler& sampler,
                                                                     bxdf::Sample&     result) const
    noexcept {
    pure_gloss_sample(Sample_base<Diffuse>::wo_, sampler, result);

    auto const coating = coating_.evaluate_f(result.wi, Sample_base<Diffuse>::wo_, result.h,
                                             result.h_dot_wi,
                                             Sample_base<Diffuse>::avoid_caustics_);

    result.reflection = coating.attenuation * result.reflection + coating.reflection;
    result.pdf        = 0.5f * (result.pdf + coating.pdf);
}

}  // namespace scene::material::substitute

#endif
