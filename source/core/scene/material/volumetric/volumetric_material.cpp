#include "volumetric_material.hpp"
#include "scene/material/collision_coefficients.inl"
#include "scene/material/material_sample.inl"
#include "scene/material/null/null_sample.hpp"
#include "scene/scene_ray.hpp"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.inl"
#include "volumetric_sample.hpp"

namespace scene::material::volumetric {

Material::Material(Sampler_settings sampler_settings) : material::Material(sampler_settings, false) {
    ior_ = 1.f;
    properties_.set(Property::Scattering_volume);
}

Material::~Material() = default;

material::Sample const& Material::sample(float3_p wo, Renderstate const& rs, Sampler& /*sampler*/,
                                         Worker& worker) const {
    if (rs.subsurface) {
        auto& sample = worker.sample<Sample>();

        sample.set_common(rs, wo, float3(0.f), float3(0.f), float2(rs.alpha()));

        float const gs = van_de_hulst_anisotropy(rs.depth);

        sample.set(gs);

        return sample;
    }

    auto& sample = worker.sample<null::Sample>();

    sample.set_common(rs, wo, float3(0.f), float3(0.f), float2(rs.alpha()));

    return sample;
}

float3 Material::prepare_sampling(Shape const& /*shape*/, uint32_t /*part*/,
                                  Transformation const& /*trafo*/, float /*area*/,
                                  Scene const& /*scene*/, Threads& /*threads*/) {
    return cc_.a * emission_;
}

size_t Material::sample_size() {
    return sizeof(Sample);
}

}  // namespace scene::material::volumetric
