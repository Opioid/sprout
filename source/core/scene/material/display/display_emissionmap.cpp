#include "display_emissionmap.hpp"
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

Emissionmap::Emissionmap(Sampler_settings const& sampler_settings, bool two_sided) noexcept
    : light::Emissionmap(sampler_settings, two_sided) {}

material::Sample const& Emissionmap::sample(float3 const& wo, Renderstate const& rs,
                                            Sampler_filter filter, sampler::Sampler& /*sampler*/,
                                            Worker const& worker, uint32_t depth) const noexcept {
    auto& sample = worker.sample<Sample>(depth);

    sample.set_basis(rs.geo_n, wo);

    sample.layer_.set_tangent_frame(rs.t, rs.b, rs.n);

    auto const& sampler = worker.sampler_2D(sampler_key(), filter);

    float3 const radiance = emission_map_.sample_3(sampler, rs.uv);
    sample.layer_.set(emission_factor_ * radiance, f0_, roughness_);

    return sample;
}

float Emissionmap::ior() const noexcept {
    return ior_;
}

size_t Emissionmap::num_bytes() const noexcept {
    return sizeof(*this);
}

void Emissionmap::set_roughness(float roughness) noexcept {
    roughness_ = roughness;
}

void Emissionmap::set_ior(float ior) noexcept {
    ior_ = ior;
    f0_  = fresnel::schlick_f0(1.f, ior);
}

}  // namespace scene::material::display
