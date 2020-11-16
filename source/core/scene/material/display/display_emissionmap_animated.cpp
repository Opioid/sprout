#include "display_emissionmap_animated.hpp"
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

Emissionmap_animated::Emissionmap_animated(Sampler_settings sampler_settings, bool two_sided)
    : light::Emissionmap_animated(sampler_settings, two_sided) {}

material::Sample const& Emissionmap_animated::sample(float3_p           wo, Ray const& /*ray*/,
                                                     Renderstate const& rs, Filter filter,
                                                     Sampler& /*sampler*/, Worker& worker) const {
    auto& sample = worker.sample<Sample>();

    sample.layer_.set_tangent_frame(rs.t, rs.b, rs.n);

    auto const& sampler = worker.sampler_2D(sampler_key(), filter);

    float3 const radiance = emission_factor_ *
                            emission_map_.sample_3(worker, sampler, rs.uv, element_);

    sample.set_common(rs, wo, radiance, radiance, alpha_);
    sample.set(fresnel::schlick_f0(ior_, rs.ior));

    return sample;
}

void Emissionmap_animated::set_roughness(float roughness) {
    float const r = ggx::clamp_roughness(roughness);

    alpha_ = r * r;
}

}  // namespace scene::material::display
