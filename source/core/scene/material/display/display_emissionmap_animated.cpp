#include "display_emissionmap_animated.hpp"
#include "base/math/distribution/distribution_2d.inl"
#include "base/math/math.hpp"
#include "base/math/vector4.inl"
#include "base/spectrum/rgb.hpp"
#include "display_sample.hpp"
#include "image/texture/texture_adapter.inl"
#include "scene/material/fresnel/fresnel.inl"
#include "scene/material/material_sample.inl"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.inl"
#include "scene/shape/shape.hpp"

namespace scene::material::display {

Emissionmap_animated::Emissionmap_animated(Sampler_settings const& sampler_settings, bool two_sided,
                                           Texture_adapter const& emission_map,
                                           float emission_factor, float animation_duration)
    : light::Emissionmap_animated(sampler_settings, two_sided, emission_map, emission_factor,
                                  animation_duration) {}

material::Sample const& Emissionmap_animated::sample(f_float3 wo, Renderstate const& rs,
                                                     Sampler_filter filter,
                                                     sampler::Sampler& /*sampler*/,
                                                     Worker const& worker, uint32_t depth) const {
    auto& sample = worker.sample<Sample>(depth);

    sample.set_basis(rs.geo_n, wo);

    sample.layer_.set_tangent_frame(rs.t, rs.b, rs.n);

    auto& sampler = worker.sampler_2D(sampler_key(), filter);

    float3 radiance = emission_map_.sample_3(sampler, rs.uv, element_);
    sample.layer_.set(emission_factor_ * radiance, f0_, roughness_);

    return sample;
}

float Emissionmap_animated::ior() const {
    return ior_;
}

size_t Emissionmap_animated::num_bytes() const {
    return sizeof(*this);
}

void Emissionmap_animated::set_roughness(float roughness) {
    roughness_ = roughness;
}

void Emissionmap_animated::set_ior(float ior) {
    ior_ = ior;
    f0_  = fresnel::schlick_f0(1.f, ior);
}

}  // namespace scene::material::display
