#include "metal_material.hpp"
#include "base/math/vector4.inl"
#include "metal_sample.hpp"
#include "scene/material/ggx/ggx.inl"
#include "scene/material/material.inl"
#include "scene/material/material_helper.hpp"
#include "scene/material/material_sample.inl"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.inl"

namespace scene::material::metal {

Material::Material(Sampler_settings sampler_settings, bool two_sided)
    : material::Material(sampler_settings, two_sided) {}

void Material::commit(Threads& /*threads*/, Scene const& /*scene*/) {
    properties_.set(Property::Caustic, alpha_[0] <= ggx::Min_alpha);
}

material::Sample const& Material::sample(float3_p wo, Renderstate const& rs, Sampler& /*sampler*/,
                                         Worker& worker) const {
    auto& sample = worker.sample<Sample>();

    auto const& sampler = worker.sampler_2D(sampler_key(), rs.filter);

    if (normal_map_.is_valid()) {
        float3 const n = sample_normal(wo, rs, normal_map_, sampler, worker.scene());
        sample.layer_.set_tangent_frame(n);
    } else {
        sample.layer_.set_tangent_frame(rs.t, rs.b, rs.n);
    }

    float rotation;
    if (rotation_map_.is_valid()) {
        rotation = (2.f * Pi) * sampler.sample_1(rotation_map_, rs.uv, worker.scene());
    } else {
        rotation = rotation_;
    }

    if (rotation > 0.f) {
        sample.layer_.rotate_tangent_frame(rotation);
    }

    sample.set_common(rs, wo, ior3_, float3(0.f), alpha_);
    sample.set(ior3_, absorption_);

    return sample;
}

void Material::set_normal_map(Texture const& normal_map) {
    normal_map_ = normal_map;
}

void Material::set_rotation_map(Texture const& rotation_map) {
    rotation_map_ = rotation_map;
}

void Material::set_ior(float3_p ior) {
    ior_  = ior[0];
    ior3_ = ior;
}

void Material::set_absorption(float3_p absorption) {
    absorption_ = absorption;
}

void Material::set_roughness(float roughness, float anisotropy) {
    float const r = ggx::clamp_roughness(roughness);

    if (anisotropy > 0.f) {
        float const rv = ggx::clamp_roughness(roughness * (1.f - anisotropy));

        alpha_ = float2(r * r, rv * rv);
    } else {
        alpha_ = float2(r * r);
    }
}

void Material::set_rotation(float angle) {
    rotation_ = (2.f * Pi) * angle;
}

size_t Material::sample_size() {
    return sizeof(Sample);
}

}  // namespace scene::material::metal
