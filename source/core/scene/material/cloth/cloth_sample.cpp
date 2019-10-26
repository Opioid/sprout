#include "cloth_sample.hpp"
#include "base/math/math.hpp"
#include "sampler/sampler.hpp"
#include "scene/material/lambert/lambert.inl"
#include "scene/material/material_sample.inl"

namespace scene::material::cloth {

float3 const& Sample::base_shading_normal() const noexcept {
    return layer_.n_;
}

float3 Sample::base_tangent_to_world(float3 const& v) const noexcept {
    return layer_.tangent_to_world(v);
}

bxdf::Result Sample::evaluate_f(float3 const& wi, bool /*include_back*/) const noexcept {
    float const n_dot_wi = layer_.clamp_n_dot(wi);
    float const pdf      = n_dot_wi * Pi_inv;
    return {pdf * diffuse_color_, pdf};
}

bxdf::Result Sample::evaluate_b(float3 const& wi, bool /*include_back*/) const noexcept {
    float const n_dot_wi = layer_.clamp_n_dot(wi);
    float const pdf      = n_dot_wi * Pi_inv;
    return {Pi_inv * diffuse_color_, pdf};
}

void Sample::sample(sampler::Sampler& sampler, bxdf::Sample& result) const noexcept {
    if (!same_hemisphere(wo_)) {
        result.pdf = 0.f;
        return;
    }

    float const n_dot_wi = lambert::Isotropic::reflect(diffuse_color_, layer_, sampler, result);
    result.reflection *= n_dot_wi;
}

void Sample::set(float3 const& color) noexcept {
    diffuse_color_ = color;
}

}  // namespace scene::material::cloth
