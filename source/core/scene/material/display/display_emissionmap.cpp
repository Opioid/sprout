#include "display_emissionmap.hpp"
#include "display_sample.hpp"
#include "image/texture/texture_adapter.inl"
#include "scene/material/fresnel/fresnel.inl"
#include "scene/material/ggx/ggx.inl"
#include "scene/material/material.inl"
#include "scene/material/material_sample.inl"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.inl"
#include "scene/shape/shape.hpp"

namespace scene::material::display {

Emissionmap::Emissionmap(Sampler_settings sampler_settings, bool two_sided)
    : light::Emissionmap(sampler_settings, two_sided) {}

material::Sample const& Emissionmap::sample(float3_p wo, Ray const& /*ray*/, Renderstate const& rs,
                                            Filter  filter, Sampler& /*sampler*/,
                                            Worker& worker) const {
    auto& sample = worker.sample<Sample>();

    sample.layer_.set_tangent_frame(rs.t, rs.b, rs.n);

    auto const& sampler = worker.sampler_2D(sampler_key(), filter);

    float3 const radiance = emission_factor_ * sampler.sample_3(emission_map_, rs.uv, worker.scene());

    sample.set_common(rs, wo, radiance, radiance, alpha_);

    sample.set(fresnel::schlick_f0(ior_, rs.ior));

    return sample;
}

void Emissionmap::set_roughness(float roughness) {
    float const r = ggx::clamp_roughness(roughness);

    alpha_ = r * r;
}

}  // namespace scene::material::display
