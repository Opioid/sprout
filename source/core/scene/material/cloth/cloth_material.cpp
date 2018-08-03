#include "cloth_material.hpp"
#include "base/math/vector4.inl"
#include "cloth_sample.hpp"
#include "image/texture/texture_adapter.inl"
#include "scene/material/material_sample.inl"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.inl"

namespace scene::material::cloth {

Material::Material(Sampler_settings const& sampler_settings, bool two_sided)
    : material::Material(sampler_settings, two_sided) {}

material::Sample const& Material::sample(float3 const& wo, Renderstate const& rs,
                                         Sampler_filter filter, sampler::Sampler& /*sampler*/,
                                         Worker const& worker, uint32_t depth) const {
    auto& sample = worker.sample<Sample>(depth);

    auto& sampler = worker.sampler_2D(sampler_key(), filter);

    sample.set_basis(rs.geo_n, wo);

    if (normal_map_.is_valid()) {
        float3 nm = normal_map_.sample_3(sampler, rs.uv);
        float3 n  = math::normalize(rs.tangent_to_world(nm));
        sample.layer_.set_tangent_frame(rs.t, rs.b, n);
    } else {
        sample.layer_.set_tangent_frame(rs.t, rs.b, rs.n);
    }

    float3 color;
    if (color_map_.is_valid()) {
        color = color_map_.sample_3(sampler, rs.uv);
    } else {
        color = color_;
    }

    sample.layer_.set(color);

    return sample;
}

float Material::ior() const {
    return 1.5f;
}

size_t Material::num_bytes() const {
    return sizeof(*this);
}

void Material::set_color_map(Texture_adapter const& color_map) {
    color_map_ = color_map;
}

void Material::set_normal_map(Texture_adapter const& normal_map) {
    normal_map_ = normal_map;
}

void Material::set_color(float3 const& color) {
    color_ = color;
}

size_t Material::sample_size() {
    return sizeof(Sample);
}

}  // namespace scene::material::cloth
