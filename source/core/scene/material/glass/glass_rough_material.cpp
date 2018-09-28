#include "glass_rough_material.hpp"
#include "base/math/vector4.inl"
#include "glass_rough_sample.hpp"
#include "image/texture/texture_adapter.inl"
#include "scene/material/collision_coefficients.inl"
#include "scene/material/ggx/ggx.inl"
#include "scene/material/material_helper.hpp"
#include "scene/material/material_sample.inl"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.inl"

namespace scene::material::glass {

Glass_rough::Glass_rough(Sampler_settings const& sampler_settings) noexcept
    : Material(sampler_settings, false) {}

material::Sample const& Glass_rough::sample(float3 const& wo, Renderstate const& rs,
                                            Sampler_filter filter, sampler::Sampler& /*sampler*/,
                                            Worker const& worker, uint32_t depth) const noexcept {
    auto& sample = worker.sample<Sample_rough>(depth);

    sample.set_basis(rs.geo_n, wo);

    auto& sampler = worker.sampler_2D(sampler_key(), filter);

    if (normal_map_.is_valid()) {
        float3 const n = sample_normal(wo, rs, normal_map_, sampler);
        sample.layer_.set_tangent_frame(n);
    } else {
        sample.layer_.set_tangent_frame(rs.t, rs.b, rs.n);
    }

    float alpha;
    if (roughness_map_.is_valid()) {
        float const r = ggx::map_roughness(roughness_map_.sample_1(sampler, rs.uv));

        alpha = r * r;
    } else {
        alpha = alpha_;
    }

    sample.set(refraction_color_, ior_, rs.ior, alpha);

    return sample;
}

float3 Glass_rough::absorption_coefficient(float2 /*uv*/, Sampler_filter /*filter*/,
                                           Worker const& /*worker*/) const noexcept {
    return absorption_coefficient_;
}

float Glass_rough::ior() const noexcept {
    return ior_;
}

bool Glass_rough::is_scattering_volume() const noexcept {
    return false;
}

size_t Glass_rough::num_bytes() const noexcept {
    return sizeof(*this);
}

void Glass_rough::set_normal_map(Texture_adapter const& normal_map) noexcept {
    normal_map_ = normal_map;
}

void Glass_rough::set_roughness_map(Texture_adapter const& roughness_map) noexcept {
    roughness_map_ = roughness_map;
}

void Glass_rough::set_refraction_color(float3 const& color) noexcept {
    refraction_color_ = color;
}

void Glass_rough::set_attenuation(float3 const& absorption_color, float distance) noexcept {
    absorption_coefficient_ = extinction_coefficient(absorption_color, distance);

    attenuation_distance_ = distance;
}

void Glass_rough::set_ior(float ior) noexcept {
    ior_ = ior;
}

void Glass_rough::set_roughness(float roughness) noexcept {
    const float r = ggx::clamp_roughness(roughness);

    alpha_ = r * r;
}

size_t Glass_rough::sample_size() noexcept {
    return sizeof(Sample_rough);
}

}  // namespace scene::material::glass
