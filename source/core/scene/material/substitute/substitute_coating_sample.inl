#ifndef SU_CORE_SCENE_MATERIAL_SUBSTITUTE_COATING_SAMPLE_INL
#define SU_CORE_SCENE_MATERIAL_SUBSTITUTE_COATING_SAMPLE_INL

#include "base/math/math.hpp"
#include "base/math/vector3.inl"
#include "scene/material/material_sample.inl"
#include "substitute_base_sample.inl"
#include "substitute_coating_sample.hpp"

namespace scene::material::substitute {

template <typename Coat, typename Diff>
bxdf::Result Sample_coating<Coat, Diff>::evaluate_f(float3_p wi) const {
    if (!same_hemisphere(wo_)) {
        return {float3(0.f), 0.f};
    }

    return evaluate<true>(wi);
}

template <typename Coat, typename Diff>
bxdf::Result Sample_coating<Coat, Diff>::evaluate_b(float3_p wi) const {
    if (!same_hemisphere(wo_)) {
        return {float3(0.f), 0.f};
    }

    return evaluate<false>(wi);
}

template <typename Coat, typename Diff>
void Sample_coating<Coat, Diff>::sample(Sampler& sampler, RNG& rng, Sample& result) const {
    if (!same_hemisphere(wo_)) {
        result.pdf = 0.f;
        return;
    }

    float       n_dot_h;
    float const f = coating_.sample(wo_, sampler, rng, n_dot_h, result);

    if (float const p = sampler.sample_1D(rng); p <= f) {
        coating_reflect(f, n_dot_h, result);
    } else {
        if (1.f == base_.metallic_) {
            pure_gloss_sample(sampler, rng, f, result);
        } else {
            float const p1 = (p - f) / (1.f - f);

            if (p1 < 0.5f) {
                diffuse_sample(sampler, rng, f, result);
            } else {
                gloss_sample(sampler, rng, f, result);
            }
        }
    }

    result.wavelength = 0.f;
}

template <typename Coat, typename Diff>
template <bool Forward>
bxdf::Result Sample_coating<Coat, Diff>::evaluate(float3_p wi) const {
    float3 const h = normalize(wo_ + wi);

    float const wo_dot_h = clamp_dot(wo_, h);

    auto const coating = coating_.evaluate_f(wi, wo_, h, wo_dot_h, avoid_caustics());

    bool const pm = 1.f == base_.metallic_;

    auto const base = pm ? base_.template pure_gloss_evaluate<Forward>(wi, wo_, h, wo_dot_h, *this)
                         : base_.template base_evaluate<Forward>(wi, wo_, h, wo_dot_h, *this);

    float const pdf = coating.f * coating.pdf + (1.f - coating.f) * base.pdf();
    return {coating.reflection + coating.attenuation * base.reflection, pdf};
}

template <typename Coat, typename Diff>
void Sample_coating<Coat, Diff>::coating_reflect(float f, float n_dot_h, Sample& result) const {
    float const n_dot_wo = coating_.clamp_abs_n_dot(wo_);

    float3 coating_attenuation;
    coating_.reflect(wo_, result.h, n_dot_wo, n_dot_h, result.h_dot_wi, result.h_dot_wi,
                     coating_attenuation, result);

    bool const pm = 1.f == base_.metallic_;

    auto const base = pm ? base_.template pure_gloss_evaluate<true>(result.wi, wo_, result.h,
                                                                    result.h_dot_wi, *this)
                         : base_.template base_evaluate<true>(result.wi, wo_, result.h,
                                                              result.h_dot_wi, *this);

    result.reflection = result.reflection + coating_attenuation * base.reflection;
    result.pdf        = f * result.pdf + (1.f - f) * base.pdf();
}

template <typename Coat, typename Diff>
void Sample_coating<Coat, Diff>::diffuse_sample(Sampler& sampler, RNG& rng, float f,
                                                Sample& result) const {
    base_.diffuse_sample(wo_, *this, sampler, rng, result);

    auto const coating = coating_.evaluate_f(result.wi, wo_, result.h, result.h_dot_wi,
                                             avoid_caustics());

    result.reflection = coating.attenuation * result.reflection + coating.reflection;
    result.pdf        = (1.f - f) * result.pdf + f * coating.pdf;
}

template <typename Coat, typename Diff>
void Sample_coating<Coat, Diff>::gloss_sample(Sampler& sampler, RNG& rng, float f,
                                              Sample& result) const {
    base_.gloss_sample(wo_, *this, sampler, rng, result);

    auto const coating = coating_.evaluate_f(result.wi, wo_, result.h, result.h_dot_wi,
                                             avoid_caustics());

    result.reflection = coating.attenuation * result.reflection + coating.reflection;
    result.pdf        = (1.f - f) * result.pdf + f * coating.pdf;
}

template <typename Coat, typename Diff>
void Sample_coating<Coat, Diff>::pure_gloss_sample(Sampler& sampler, RNG& rng, float f,
                                                   Sample& result) const {
    base_.pure_gloss_sample(wo_, *this, sampler, rng, result);

    auto const coating = coating_.evaluate_f(result.wi, wo_, result.h, result.h_dot_wi,
                                             avoid_caustics());

    result.reflection = coating.attenuation * result.reflection + coating.reflection;
    result.pdf        = (1.f - f) * result.pdf + f * coating.pdf;
}

}  // namespace scene::material::substitute

#endif
