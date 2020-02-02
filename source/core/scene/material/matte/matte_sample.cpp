#include "matte_sample.hpp"
#include "sampler/sampler.hpp"
#include "scene/material/disney/disney.inl"
#include "scene/material/material_sample.inl"
// #include "scene/material/lambert/lambert.inl"
// #include "scene/material/oren_nayar/oren_nayar.inl"
#include "base/math/vector4.inl"

namespace scene::material::matte {

float3 const& Sample::base_shading_normal() const {
    return layer_.n_;
}

bxdf::Result Sample::evaluate_f(float3 const& wi, bool) const {
    float const n_dot_wi = layer_.clamp_n_dot(wi);
    float const n_dot_wo = layer_.clamp_abs_n_dot(wo_);

    float3 const h = normalize(wo_ + wi);

    float const h_dot_wi = clamp_dot(h, wi);

    auto const brdf = disney::Isotropic::reflection(h_dot_wi, n_dot_wi, n_dot_wo, alpha_,
                                                    diffuse_color_);
    return {n_dot_wi * brdf.reflection, brdf.pdf};
}

bxdf::Result Sample::evaluate_b(float3 const& wi, bool) const {
    float const n_dot_wi = layer_.clamp_n_dot(wi);
    float const n_dot_wo = layer_.clamp_abs_n_dot(wo_);

    float3 const h = normalize(wo_ + wi);

    float const h_dot_wi = clamp_dot(h, wi);

    auto const brdf = disney::Isotropic::reflection(h_dot_wi, n_dot_wi, n_dot_wo, alpha_,
                                                    diffuse_color_);
    return brdf;
}

void Sample::sample(sampler::Sampler& sampler, bxdf::Sample& result) const {
    float const n_dot_wo = layer_.clamp_abs_n_dot(wo_);

    float2 const xi = sampler.generate_sample_2D();

    float const n_dot_wi = disney::Isotropic::reflect(wo_, n_dot_wo, layer_, alpha_, diffuse_color_,
                                                      xi, result);

    result.reflection *= n_dot_wi;

    result.wavelength = 0.f;
}

void Sample::set(float3 const& color) {
    diffuse_color_ = color;
    alpha_         = math::pow2(1.f);
}

}  // namespace scene::material::matte
