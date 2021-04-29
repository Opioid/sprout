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

Material_isotropic::Material_isotropic(Sampler_settings sampler_settings, bool two_sided)
    : Material(sampler_settings, two_sided) {}

void Material_isotropic::commit(Threads& /*threads*/, Scene const& /*scene*/) {
    properties_.set(Property::Caustic, alpha_ <= ggx::Min_alpha);
}

material::Sample const& Material_isotropic::sample(float3_p           wo, Ray const& /*ray*/,
                                                   Renderstate const& rs, Filter filter,
                                                   Sampler& /*sampler*/, Worker& worker) const {
    auto& sample = worker.sample<Sample_isotropic>();

    if (normal_map_.is_valid()) {
        auto const&  sampler = worker.sampler_2D(sampler_key(), filter);
        float3 const n       = sample_normal(wo, rs, normal_map_, sampler, worker.scene());
        sample.layer_.set_tangent_frame(n);
    } else {
        sample.layer_.set_tangent_frame(rs.t, rs.b, rs.n);
    }

    sample.set_common(rs, wo, ior3_, float3(0.), alpha_);
    sample.set(ior3_, absorption_);

    return sample;
}

void Material_isotropic::set_normal_map(Turbotexture const& normal_map) {
    normal_map_ = normal_map;
}

void Material_isotropic::set_ior(float3_p ior) {
    ior_  = ior[0];
    ior3_ = ior;
}

void Material_isotropic::set_absorption(float3_p absorption) {
    absorption_ = absorption;
}

void Material_isotropic::set_roughness(float roughness) {
    float const r = ggx::clamp_roughness(roughness);

    alpha_ = r * r;
}

size_t Material_isotropic::sample_size() {
    return sizeof(Sample_isotropic);
}

Material_anisotropic::Material_anisotropic(Sampler_settings sampler_settings, bool two_sided)
    : Material(sampler_settings, two_sided) {}

void Material_anisotropic::commit(Threads& /*threads*/, Scene const& /*scene*/) {
    properties_.set(Property::Caustic, alpha_[0] <= ggx::Min_alpha || alpha_[1] <= ggx::Min_alpha);
}

material::Sample const& Material_anisotropic::sample(float3_p           wo, Ray const& /*ray*/,
                                                     Renderstate const& rs, Filter filter,
                                                     Sampler& /*sampler*/, Worker& worker) const {
    auto& sample = worker.sample<Sample_anisotropic>();

    sample.set_common(rs, wo, ior3_, float3(0.f), alpha_[0]);

    auto& sampler = worker.sampler_2D(sampler_key(), filter);

    if (normal_map_.is_valid()) {
        float3 const n = sample_normal(wo, rs, normal_map_, sampler, worker.scene());
        sample.layer_.set_tangent_frame(n);
    } else if (direction_map_.is_valid()) {
        float2 tm = sampler.sample_2(direction_map_, rs.uv, worker.scene());
        float3 t  = normalize(rs.tangent_to_world(tm));
        float3 b  = cross(rs.n, t);

        sample.layer_.set_tangent_frame(t, b, rs.n);
    } else {
        sample.layer_.set_tangent_frame(rs.t, rs.b, rs.n);
    }

    sample.set(ior3_, absorption_, alpha_);

    return sample;
}

void Material_anisotropic::set_normal_map(Turbotexture const& normal_map) {
    normal_map_ = normal_map;
}

void Material_anisotropic::set_direction_map(Turbotexture const& direction_map) {
    direction_map_ = direction_map;
}

void Material_anisotropic::set_ior(float3_p ior) {
    ior_  = ior[0];
    ior3_ = ior;
}

void Material_anisotropic::set_absorption(float3_p absorption) {
    absorption_ = absorption;
}

void Material_anisotropic::set_roughness(float2 roughness) {
    float2 const r = float2(ggx::clamp_roughness(roughness[0]), ggx::clamp_roughness(roughness[1]));

    alpha_ = r * r;
}

size_t Material_anisotropic::sample_size() {
    return sizeof(Sample_anisotropic);
}

}  // namespace scene::material::metal
