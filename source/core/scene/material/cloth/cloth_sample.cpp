#include "cloth_sample.hpp"
#include "base/math/math.hpp"
#include "sampler/sampler.hpp"
#include "scene/material/lambert/lambert.inl"
#include "scene/material/material_sample.inl"

namespace scene::material::cloth {

const material::Layer& Sample::base_layer() const noexcept {
    return layer_;
}

bxdf::Result Sample::evaluate(float3 const& wi) const noexcept {
    float const n_dot_wi = layer_.clamp_n_dot(wi);
    float const pdf      = n_dot_wi * math::Pi_inv;
    return {pdf * diffuse_color_, pdf};
}

void Sample::sample(sampler::Sampler& sampler, bxdf::Sample& result) const noexcept {
    if (!same_hemisphere(wo_)) {
        result.pdf = 0.f;
        return;
    }

    float const n_dot_wi = lambert::Isotropic::reflect(diffuse_color_, layer_, sampler,
                                                       result);
    result.reflection *= n_dot_wi;
}

void Sample::set(float3 const& color) noexcept {
    diffuse_color_ = color;
}

}  // namespace scene::material::cloth
