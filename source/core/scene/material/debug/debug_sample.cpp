#include "debug_sample.hpp"
#include "base/math/sampling.inl"
#include "sampler/sampler.hpp"
#include "scene/material/bxdf.hpp"
#include "scene/material/material_sample.inl"

namespace scene::material::debug {

bxdf::Result Sample::evaluate(float3_p wi) const {
    float const n_dot_wi = layer_.clamp_n_dot(wi);

    float const pdf = n_dot_wi * Pi_inv;

    float3 const lambert = Pi_inv * albedo_;

    return {n_dot_wi * lambert, pdf};
}

void Sample::sample(Sampler& sampler, RNG& rng, bxdf::Sample& result) const {
    float2 const s2d = sampler.sample_2D(rng);

    float3 const is = sample_hemisphere_cosine(s2d);

    float3 const wi = normalize(layer_.tangent_to_world(is));

    float const n_dot_wi = layer_.clamp_n_dot(wi);

    result.reflection = n_dot_wi * Pi_inv * albedo_;
    result.wi         = wi;
    result.pdf        = n_dot_wi * Pi_inv;
    result.wavelength = 0.f;
    result.type.clear(bxdf::Type::Diffuse_reflection);
}

}  // namespace scene::material::debug
