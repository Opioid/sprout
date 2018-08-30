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

 //   auto const base = layer_.base_evaluate(wi, wo_, h, wo_dot_h, avoid_caustics_);

//    float const pdf = (coating.pdf + 2.f * base.pdf) / 3.f;
//    return {coating.reflection + coating.attenuation * base.reflection, pdf};

    float3 n = layer_.n_;

    float eta_i = coating_.ior_ / layer_.ior_;
    float eta_t = layer_.ior_ / coating_.ior_;

    float3 wo1 = wo_;
    float3 wi1 = wi;

    {
        std::swap(eta_i, eta_t);

        float const n_dot_wo = std::min(std::abs(math::dot(n, wo_)), 1.f);
        float const sint2    = (eta_i * eta_i) * (1.f - n_dot_wo * n_dot_wo);

        float n_dot_t;
        if (sint2 >= 1.f) {
            n_dot_t = 0.f;
            return {float3(0.f), 0.f};
        } else {
            n_dot_t = std::sqrt(1.f - sint2);
        }

        wo1 = -math::normalize((eta_i * n_dot_wo - n_dot_t) * n - eta_i * wo_);
    }

    {
    //    std::swap(eta_i, eta_t);

        float const n_dot_wi = std::min(std::abs(math::dot(n, wi)), 1.f);
        float const sint2    = (eta_i * eta_i) * (1.f - n_dot_wi * n_dot_wi);

        float n_dot_t;
        if (sint2 >= 1.f) {
            n_dot_t = 0.f;
            return {float3(0.f), 0.f};
        } else {
            n_dot_t = std::sqrt(1.f - sint2);
        }

        wi1 = -math::normalize((eta_i * n_dot_wi - n_dot_t) * n - eta_i * wi);
    }

    float3 const h1 = math::normalize(wo1 + wi);

    float const wo1_dot_h1 = clamp_dot(wo1, h1);

    auto const base = layer_.base_evaluate(wi1, wo1, h1, wo1_dot_h1, avoid_caustics_);

    float const pdf = base.pdf;
    return {base.reflection, pdf};
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
