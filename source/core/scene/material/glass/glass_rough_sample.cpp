#include "glass_rough_sample.hpp"
#include "base/math/math.hpp"
#include "base/math/vector3.inl"
#include "sampler/sampler.hpp"
#include "scene/material/bxdf.hpp"
#include "scene/material/collision_coefficients.inl"
#include "scene/material/fresnel/fresnel.inl"
#include "scene/material/ggx/ggx.inl"
#include "scene/material/material_sample.inl"

#include "base/debug/assert.hpp"
#include "scene/material/material_test.hpp"

#include <iostream>

namespace scene::material::glass {

const material::Layer& Sample_rough::base_layer() const noexcept {
    return layer_;
}

bxdf::Result Sample_rough::evaluate(float3 const& wi) const noexcept {
    if (!same_hemisphere(wo_)) {
        return {float3(0.f), 0.f};
        IoR ior = ior_.swapped();

        Layer tmp_layer = layer_;
        tmp_layer.n_    = -layer_.n_;

        float const n_dot_wi = tmp_layer.clamp_reverse_n_dot(wi);

        float3 const h = math::normalize(ior.eta_t * wi + ior.eta_i * wo_);

        float const eta = ior.eta_i / ior.eta_t;

        float const wo_dot_h = clamp_abs_dot(wo_, h);

        float const sint2 = (eta * eta) * (1.f - wo_dot_h * wo_dot_h);

        if (sint2 >= 1.f) {
            return {float3(0.f), 0.f};
        }

        fresnel::Schlick1 const schlick(f0_);

        auto const ggx = ggx::Isotropic::refraction2(wi, wo_, h, tmp_layer, alpha_, ior, schlick,
                                                     0.f);

        return {n_dot_wi * ggx.reflection, 0.5f * ggx.pdf};
    } else {
        float const n_dot_wi = layer_.clamp_n_dot(wi);
        float const n_dot_wo = layer_.clamp_abs_n_dot(wo_);

        float3 const h = math::normalize(wo_ + wi);

        float const wo_dot_h = clamp_dot(wo_, h);
        float const n_dot_h  = math::saturate(math::dot(layer_.n_, h));

        fresnel::Schlick const schlick(f0_);
        auto const ggx = ggx::Isotropic::reflection(n_dot_wi, n_dot_wo, wo_dot_h, n_dot_h, alpha_,
                                                    schlick);

        return {n_dot_wi * ggx.reflection, ggx.pdf};
    }
}

void Sample_rough::sample(sampler::Sampler& sampler, bxdf::Sample& result) const noexcept {
    bool const same_side = same_hemisphere(wo_);

    Layer layer = layer_;
    IoR   ior;

    if (same_side) {
        ior = ior_;
    } else {
        layer.n_ = -layer_.n_;
        ior      = ior_.swapped();
    }

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

        fresnel::Schlick1 const schlick(f0_);
        f = schlick(cos_x);
    }

    if (sampler.generate_sample_1D() <= f) {
        float const n_dot_wi = ggx::Isotropic::reflect(wo_, h, n_dot_wo, n_dot_h, wi_dot_h,
                                                       wo_dot_h, layer, alpha_, result);
        result.reflection *= n_dot_wi;
    } else {
        float const n_dot_wi = ggx::Isotropic::refract(wo_, h, n_dot_wo, n_dot_h, wi_dot_h,
                                                       wo_dot_h, layer, alpha_, ior, result);
        result.reflection *= n_dot_wi * color_;
    }

    result.wavelength = 0.f;
}

void Sample_rough::set(float3 const& refraction_color, float ior, float ior_outside,
                       float alpha) noexcept {
    color_ = refraction_color;

    f0_ = fresnel::schlick_f0(ior, ior_outside);

    alpha_ = alpha;

    ior_.eta_t = ior;
    ior_.eta_i = ior_outside;
}

}  // namespace scene::material::glass
