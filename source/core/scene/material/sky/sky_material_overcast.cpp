#include "sky_material_overcast.hpp"
#include "base/math/vector3.inl"
#include "scene/material/material_sample.inl"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.inl"

namespace scene::material::sky {

Material_overcast::Material_overcast(Sampler_settings const& sampler_settings, bool two_sided)
    : Material(sampler_settings, two_sided) {}

material::Sample const& Material_overcast::sample(float3 const&      wo, Ray const& /*ray*/,
                                                  Renderstate const& rs, Filter /*filter*/,
                                                  sampler::Sampler& /*sampler*/,
                                                  Worker const& worker) const {
    auto& sample = worker.sample<light::Sample>();

    sample.set_basis(rs.geo_n, wo);

    sample.set(overcast(-wo));

    return sample;
}

float3 Material_overcast::evaluate_radiance(float3 const& wi, float2 /*uv*/, float /*area*/,
                                            Filter /*filter*/, Worker const& /*worker*/) const {
    return overcast(wi);
}

float3 Material_overcast::average_radiance(float /*area*/, Scene const& /*scene*/) const {
    if (is_two_sided()) {
        return 2.f * color_;
    }

    return color_;
}

float Material_overcast::ior() const {
    return 1.5f;
}

size_t Material_overcast::num_bytes() const {
    return sizeof(*this);
}

void Material_overcast::set_emission(float3 const& radiance) {
    color_ = radiance;
}

float3 Material_overcast::overcast(float3 const& wi) const {
    return ((1.f + 2.f * dot(float3(0.f, 1.f, 0.f), wi)) / 3.f) * color_;
}

}  // namespace scene::material::sky
