#ifndef SU_CORE_SCENE_MATERIAL_SUBSTITUTE_COATING_SAMPLE_INL
#define SU_CORE_SCENE_MATERIAL_SUBSTITUTE_COATING_SAMPLE_INL

#include "base/math/math.hpp"
#include "base/math/vector3.inl"
#include "scene/material/material_sample.inl"
#include "substitute_base_sample.inl"
#include "substitute_coating_sample.hpp"

namespace scene::material::substitute {

template <typename Coating>
bxdf::Result Sample_coating<Coating>::evaluate(float3 const& wi) const noexcept {
    if (!same_hemisphere(wo_)) {
        return {float3::identity(), 0.f};
    }

    float3 const h = math::normalize(wo_ + wi);

    float const wo_dot_h = clamp_dot(wo_, h);

    auto const coating = coating_.evaluate(wi, wo_, h, wo_dot_h, avoid_caustics_);

    if (1.f == layer_.metallic_) {
        auto const base = layer_.pure_gloss_evaluate(wi, wo_, h, wo_dot_h, avoid_caustics_);

        float const pdf = (coating.pdf + base.pdf) * 0.5f;
        return {coating.reflection + coating.attenuation * base.reflection, pdf};
    }

    auto const base = layer_.base_evaluate(wi, wo_, h, wo_dot_h, avoid_caustics_);

    float const pdf = (coating.pdf + 2.f * base.pdf) / 3.f;
    return {coating.reflection + coating.attenuation * base.reflection, pdf};

    /*
    float3 const h = math::normalize(wo_ + wi);

    float const wo_dot_h = clamp_dot(wo_, h);

    float3 n = coating_.n_;

    float eta0 = coating_.ior_ / layer_.ior_;
    float eta1 = layer_.ior_ / coating_.ior_;

 //   float const a = math::pow2(coating_.ior_) / math::pow2(layer_.ior_);

    float3 wo1 = wo_;
    float3 wi1 = wi;

    bool const ro = refract(n, wo_, eta0, wo1);
    bool const ri = refract(n, wi, eta1, wi1);


    auto const coating = coating_.evaluate(wi, wo_, h, wi1, wo1, wo_dot_h, avoid_caustics_);

    if (!ri || !ro) {
        return {coating.reflection, coating.pdf / 3.f};
    }

    float3 const h1 = math::normalize(wo1 + wi1);

    if (1.f == layer_.metallic_) {
        auto const base = layer_.pure_gloss_evaluate(wi1, wo1, h1, wo_dot_h, avoid_caustics_);

        float const pdf = (coating.pdf + base.pdf) * 0.5f;
        return {coating.reflection + coating.attenuation * base.reflection, pdf};
    }

    float const wo1_dot_h1 = clamp_dot(wo1, h1);

    auto const base = layer_.base_evaluate(wi1, wo1, h1, wo1_dot_h1, avoid_caustics_);

    float const pdf = (coating.pdf + 2.f * base.pdf) / 3.f;
    return {coating.reflection + coating.attenuation * base.reflection, pdf};
    */
}

template <typename Coating>
void Sample_coating<Coating>::sample(sampler::Sampler& sampler, bxdf::Sample& result) const
    noexcept {
    if (!same_hemisphere(wo_)) {
        result.pdf = 0.f;
        return;
    }

    float const p = sampler.generate_sample_1D();

    if (p < 0.5f) {
        float3 coating_attenuation;
        coating_.sample(wo_, sampler, coating_attenuation, result);

        if (1.f == layer_.metallic_) {
            auto const base = layer_.pure_gloss_evaluate(result.wi, wo_, result.h, result.h_dot_wi,
                                                         avoid_caustics_);

            result.reflection = result.reflection + coating_attenuation * base.reflection;
            result.pdf        = (result.pdf + base.pdf) * 0.5f;
        } else {
            auto const base = layer_.base_evaluate(result.wi, wo_, result.h, result.h_dot_wi,
                                                   avoid_caustics_);

            result.reflection = result.reflection + coating_attenuation * base.reflection;
            result.pdf        = (result.pdf + 2.f * base.pdf) * 0.5f;
        }
    } else {
        if (1.f == layer_.metallic_) {
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

template <typename Coating>
void Sample_coating<Coating>::diffuse_sample_and_coating(sampler::Sampler& sampler,
                                                         bxdf::Sample&     result) const noexcept {
    layer_.diffuse_sample(wo_, sampler, avoid_caustics_, result);

    auto const coating = coating_.evaluate(result.wi, wo_, result.h, result.h_dot_wi,
                                           avoid_caustics_);

    result.reflection = coating.attenuation * result.reflection + coating.reflection;
    result.pdf        = (2.f * result.pdf + coating.pdf) * 0.25f;
}

template <typename Coating>
void Sample_coating<Coating>::gloss_sample_and_coating(sampler::Sampler& sampler,
                                                       bxdf::Sample&     result) const noexcept {
    layer_.gloss_sample(wo_, sampler, result);

    auto const coating = coating_.evaluate(result.wi, wo_, result.h, result.h_dot_wi,
                                           avoid_caustics_);

    result.reflection = coating.attenuation * result.reflection + coating.reflection;
    result.pdf        = (2.f * result.pdf + coating.pdf) * 0.25f;
}

template <typename Coating>
void Sample_coating<Coating>::pure_gloss_sample_and_coating(sampler::Sampler& sampler,
                                                            bxdf::Sample& result) const noexcept {
    layer_.pure_gloss_sample(wo_, sampler, result);

    auto const coating = coating_.evaluate(result.wi, wo_, result.h, result.h_dot_wi,
                                           avoid_caustics_);

    result.reflection = coating.attenuation * result.reflection + coating.reflection;
    result.pdf        = 0.5f * (result.pdf + coating.pdf);
}

}  // namespace scene::material::substitute

#endif
