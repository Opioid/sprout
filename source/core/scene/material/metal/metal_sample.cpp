#include "metal_sample.hpp"
#include "base/math/math.hpp"
#include "base/math/vector3.inl"
#include "sampler/sampler.hpp"
#include "scene/material/ggx/ggx.inl"
#include "scene/material/material_sample.inl"

namespace scene::material::metal {

const material::Sample::Layer& Sample_isotropic::base_layer() const noexcept {
    return layer_;
}

bxdf::Result Sample_isotropic::evaluate(float3 const& wi) const noexcept {
    if (!same_hemisphere(wo_) || (avoid_caustics_ && layer_.alpha_ <= ggx::Min_alpha)) {
        return {float3::identity(), 0.f};
    }

    float const n_dot_wi = layer_.clamp_n_dot(wi);
    float const n_dot_wo = layer_.clamp_abs_n_dot(wo_);

    float3 const h = math::normalize(wo_ + wi);

    float const wo_dot_h = clamp_dot(wo_, h);

    float const n_dot_h = math::saturate(math::dot(layer_.n_, h));

    const fresnel::Conductor conductor(layer_.ior_, layer_.absorption_);
    auto const ggx = ggx::Isotropic::reflection(n_dot_wi, n_dot_wo, wo_dot_h, n_dot_h, layer_,
                                                conductor);

    return {n_dot_wi * ggx.reflection, ggx.pdf};
}

void Sample_isotropic::sample(sampler::Sampler& sampler, bxdf::Sample& result) const noexcept {
    if (!same_hemisphere(wo_)) {
        result.pdf = 0.f;
        return;
    }

    float const n_dot_wo = layer_.clamp_abs_n_dot(wo_);  // layer_.clamp_n_dot(wo_);

    const fresnel::Conductor conductor(layer_.ior_, layer_.absorption_);
    float const n_dot_wi = ggx::Isotropic::reflect(wo_, n_dot_wo, layer_, conductor, sampler,
                                                   result);
    result.reflection *= n_dot_wi;

    result.wavelength = 0.f;
}

void Sample_isotropic::Layer::set(float3 const& ior, float3 const& absorption,
                                  float alpha) noexcept {
    ior_        = ior;
    absorption_ = absorption;
    alpha_      = alpha;
}

const material::Sample::Layer& Sample_anisotropic::base_layer() const noexcept {
    return layer_;
}

bxdf::Result Sample_anisotropic::evaluate(float3 const& wi) const noexcept {
    if (!same_hemisphere(wo_)) {
        return {float3::identity(), 0.f};
    }

    float const n_dot_wi = layer_.clamp_n_dot(wi);
    float const n_dot_wo = layer_.clamp_abs_n_dot(wo_);  // layer_.clamp_n_dot(wo_);

    float3 const h = math::normalize(wo_ + wi);

    float const wo_dot_h = clamp_dot(wo_, h);

    const fresnel::Conductor conductor(layer_.ior_, layer_.absorption_);
    auto const ggx = ggx::Anisotropic::reflection(h, n_dot_wi, n_dot_wo, wo_dot_h, layer_,
                                                  conductor);

    return {n_dot_wi * ggx.reflection, ggx.pdf};
}

void Sample_anisotropic::sample(sampler::Sampler& sampler, bxdf::Sample& result) const noexcept {
    if (!same_hemisphere(wo_)) {
        result.pdf = 0.f;
        return;
    }

    float const n_dot_wo = layer_.clamp_abs_n_dot(wo_);  // layer_.clamp_n_dot(wo_);

    const fresnel::Conductor conductor(layer_.ior_, layer_.absorption_);
    float const n_dot_wi = ggx::Anisotropic::reflect(wo_, n_dot_wo, layer_, conductor, sampler,
                                                     result);
    result.reflection *= n_dot_wi;

    result.wavelength = 0.f;
}

void Sample_anisotropic::Layer::set(float3 const& ior, float3 const& absorption,
                                    float2 roughness) noexcept {
    ior_        = ior;
    absorption_ = absorption;

    float2 const a = roughness * roughness;
    a_             = a;
    alpha2_        = a * a;
    axy_           = a[0] * a[1];
}

}  // namespace scene::material::metal
