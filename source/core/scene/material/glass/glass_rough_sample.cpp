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

namespace scene::material::glass {

const material::Layer& Sample_rough::base_layer() const noexcept {
    return layer_;
}

bxdf::Result Sample_rough::evaluate(float3 const& wi) const noexcept {
    if (ior_.eta_i == ior_.eta_t) {
        return {float3(0.f), 0.f};
    }

    if (!same_hemisphere(wo_)) {
        IoR ior = ior_.swapped();

        float3 const h = -math::normalize(ior.eta_t * wi + ior.eta_i * wo_);

        float const wi_dot_h = math::dot(wi, h);
        if (wi_dot_h <= 0.f) {
            return {float3(0.f), 0.f};
        }

        float const eta = ior.eta_i / ior.eta_t;

        float const wo_dot_h = math::dot(wo_, h);

        float const sint2 = (eta * eta) * (1.f - wo_dot_h * wo_dot_h);

        if (sint2 >= 1.f) {
            return {float3(0.f), 0.f};
        }

        float const n_dot_wi = layer_.clamp_n_dot(wi);
        float const n_dot_wo = layer_.clamp_abs_n_dot(wo_);
        float const n_dot_h  = math::saturate(math::dot(layer_.n_, h));

        fresnel::Schlick1 const schlick(f0_);

        auto const ggx = ggx::Isotropic::refraction(n_dot_wi, n_dot_wo, wi_dot_h, wo_dot_h, n_dot_h,
                                                    alpha_, ior, schlick);

        return {std::min(n_dot_wi, n_dot_wo) * ggx.reflection, ggx.pdf};
    } else {
        float const n_dot_wi = layer_.clamp_n_dot(wi);
        float const n_dot_wo = layer_.clamp_abs_n_dot(wo_);

        float3 const h = math::normalize(wo_ + wi);

        float const wo_dot_h = clamp_dot(wo_, h);
        float const n_dot_h  = math::saturate(math::dot(layer_.n_, h));

        fresnel::Schlick const schlick(f0_);

        float3     fresnel;
        auto const ggx = ggx::Isotropic::reflection(n_dot_wi, n_dot_wo, wo_dot_h, n_dot_h, alpha_,
                                                    schlick, fresnel);

        return {n_dot_wi * ggx.reflection, fresnel[0] * ggx.pdf};
    }
}

void Sample_rough::sample(sampler::Sampler& sampler, bxdf::Sample& result) const noexcept {
    if (ior_.eta_i == ior_.eta_t) {
        result.reflection = color_;
        result.wi         = -wo_;
        result.pdf        = 1.f;
        result.wavelength = 0.f;
        result.type.clear(bxdf::Type::Specular_transmission);
        return;
    }

    bool const same_side = same_hemisphere(wo_);

    Layer const layer = layer_.swapped(same_side);

    IoR const ior = ior_.swapped(same_side);

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

        f = fresnel::schlick(cos_x, f0_);
    }

    if (sampler.generate_sample_1D() <= f) {
        float const n_dot_wi = ggx::Isotropic::reflect(wo_, h, n_dot_wo, n_dot_h, wi_dot_h,
                                                       wo_dot_h, layer, alpha_, result);

        result.reflection *= f * n_dot_wi;
        result.pdf *= f;
    } else {
        float const r_wo_dot_h = same_side ? -wo_dot_h : wo_dot_h;

        float const n_dot_wi = ggx::Isotropic::refract(wo_, h, n_dot_wo, n_dot_h, -wi_dot_h,
                                                       r_wo_dot_h, layer, alpha_, ior, result);

        float const omf = 1.f - f;

        result.reflection *= omf * n_dot_wi * color_;
        result.pdf *= omf;
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
