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

const material::Sample::Layer& Sample_rough::base_layer() const noexcept {
    return layer_;
}

bxdf::Result Sample_rough::evaluate(float3 const& wi) const noexcept {
    if (!same_hemisphere(wo_)) {
     //   return {float3(0.f), 0.f};
        float3 n = layer_.n_;

        float eta_i = ior_.eta_i;
        float eta_t = ior_.eta_t;

        if (!same_hemisphere(wo_)) {
            n = -n;

            std::swap(eta_i, eta_t);
        }

        //    float const n_dot_wo = std::min(std::abs(math::dot(n, wo_)), 1.f);

        float const n_dot_wi = layer_.clamp_n_dot(wi);

        float const n_dot_wo = layer_.clamp_abs_n_dot(wo_);

        float const eta = eta_i / eta_t;

        float3 const h = math::normalize(eta_t * wi + eta_i * wo_);

        IoR tmp_ior;

        tmp_ior.eta_t = ior_.eta_i;
        tmp_ior.eta_i = ior_.eta_t;

        //     float const eta = tmp_ior.eta_i_ / tmp_ior.eta_t_;

        //  float3 const h = math::normalize(wo + wi);
        //  float3 const h = math::normalize(tmp_ior.eta_t * wo + tmp_ior.eta_i * wi);

        //     std::cout <<  "h: " << h << std::endl;

        float const wi_dot_h = clamp_abs_dot(wi, h);
        //   float const wo_dot_h = clamp_abs_dot(-wo_, h);
        float const wo_dot_h = clamp_abs_dot(wo_, h);
        float const n_dot_h  = math::saturate(-math::dot(layer_.n_, h));

        float const sint2 = (eta * eta) * (1.f - wo_dot_h * wo_dot_h);

        if (sint2 >= 1.f) {
            return {float3(0.f), 0.f};
        }

        fresnel::Schlick1 const schlick(layer_.f0_);
//        auto const ggx = ggx::Isotropic::refraction(n_dot_wi, n_dot_wo, wi_dot_h, wo_dot_h, n_dot_h,
//                                                    layer_, tmp_ior, schlick);


        auto const ggx = ggx::Isotropic::refraction2(wi, wo_, h,
                                                    layer_, tmp_ior, schlick);

        return {n_dot_wi * ggx.reflection, 0.5f * ggx.pdf};
    } else {
        float const n_dot_wi = layer_.clamp_n_dot(wi);
        float const n_dot_wo = layer_.clamp_abs_n_dot(wo_);

        float3 const h = math::normalize(wo_ + wi);

        float const wo_dot_h = clamp_dot(wo_, h);
        float const n_dot_h  = math::saturate(math::dot(layer_.n_, h));

        fresnel::Schlick const schlick(layer_.f0_);
        auto const ggx = ggx::Isotropic::reflection(n_dot_wi, n_dot_wo, wo_dot_h, n_dot_h, layer_,
                                                    schlick);

        return {n_dot_wi * ggx.reflection, 0.5f * ggx.pdf};
    }
}

void Sample_rough::sample(sampler::Sampler& sampler, bxdf::Sample& result) const noexcept {
    //	float const p = sampler.generate_sample_1D();

    //	if (p < 0.5f) {
    //		float const n_dot_wi = BSDF::reflect(*this, layer_, sampler, result);
    //		result.pdf *= 0.5f;
    //		result.reflection *= n_dot_wi;

    //	} else {
    //		float const n_dot_wi = BSDF::refract(*this, layer_, sampler, result);
    //		result.pdf *= 0.5f;
    //		result.reflection *= n_dot_wi;
    //	}

    //	result.wavelength = 0.f;

    bool const same_side = same_hemisphere(wo_);

    float const p = sampler.generate_sample_1D();

    if (same_side) {
        if (p < 0.5f) {
            refract(same_side, layer_, sampler, result);
        } else {
            reflect(layer_, sampler, result);
        }
    } else {
        Layer tmp_layer = layer_;
        tmp_layer.n_    = -layer_.n_;

        if (p < 0.5f) {
            refract(same_side, tmp_layer, sampler, result);
        } else {
            reflect_internally(tmp_layer, sampler, result);
        }
    }

    result.pdf *= 0.5f;
    result.wavelength = 0.f;
}

void Sample_rough::set(float3 const& refraction_color, float3 const& absorption_color,
                       float attenuation_distance, float ior, float ior_outside,
                       float alpha) noexcept {
    layer_.color_ = refraction_color;

    layer_.absorption_coefficient_ = material::extinction_coefficient(absorption_color,
                                                                      attenuation_distance);

    layer_.f0_    = fresnel::schlick_f0(ior, ior_outside);
    layer_.alpha_ = alpha;

    ior_.eta_t = ior;
    ior_.eta_i = ior_outside;
}

void Sample_rough::reflect(Layer const& layer, sampler::Sampler& sampler,
                           bxdf::Sample& result) const noexcept {
    float const n_dot_wo = layer.clamp_abs_n_dot(wo_);

    fresnel::Schlick const schlick(layer.f0_);
    float const n_dot_wi = ggx::Isotropic::reflect(wo_, n_dot_wo, layer, schlick, sampler, result);

    SOFT_ASSERT(testing::check(result, wo_, layer));

    result.reflection *= n_dot_wi;
}

void Sample_rough::reflect_internally(Layer const& layer, sampler::Sampler& sampler,
                                      bxdf::Sample& result) const noexcept {
    IoR const ior = ior_.swapped();

    float const n_dot_wo = layer.clamp_abs_n_dot(wo_);

    fresnel::Schlick1 const schlick(layer.f0_);
    float const n_dot_wi = ggx::Isotropic::reflect_internally(wo_, n_dot_wo, layer, ior, schlick,
                                                              sampler, result);

    SOFT_ASSERT(testing::check(result, wo_, layer));

    result.reflection *= n_dot_wi;
}

void Sample_rough::refract(bool same_side, Layer const& layer, sampler::Sampler& sampler,
                           bxdf::Sample& result) const noexcept {
    IoR const ior = ior_.swapped(same_side);

    float const n_dot_wo = layer.clamp_abs_n_dot(wo_);

    fresnel::Schlick1 const schlick(layer.f0_);
    float const n_dot_wi = ggx::Isotropic::refract(wo_, n_dot_wo, layer, ior, schlick, sampler,
                                                   result);

    result.reflection *= n_dot_wi * layer.color_;
}

}  // namespace scene::material::glass
