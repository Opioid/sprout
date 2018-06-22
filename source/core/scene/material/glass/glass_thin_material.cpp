#include "glass_thin_material.hpp"
#include "base/math/vector4.inl"
#include "glass_thin_sample.hpp"
#include "image/texture/texture_adapter.inl"
#include "rendering/integrator/integrator_helper.hpp"
#include "scene/material/material_attenuation.inl"
#include "scene/material/material_helper.hpp"
#include "scene/material/material_sample.inl"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.inl"

namespace scene::material::glass {

Glass_thin::Glass_thin(Sampler_settings const& sampler_settings)
    : Material(sampler_settings, true) {}

material::Sample const& Glass_thin::sample(f_float3 wo, Renderstate const& rs,
                                           Sampler_filter filter, sampler::Sampler& /*sampler*/,
                                           Worker const& worker, uint32_t depth) const {
    auto& sample = worker.sample<Sample_thin>(depth);

    sample.set_basis(rs.geo_n, wo);

    if (normal_map_.is_valid()) {
        auto&        sampler = worker.sampler_2D(sampler_key(), filter);
        float3 const n       = sample_normal(wo, rs, normal_map_, sampler);
        sample.layer_.set_tangent_frame(n);
    } else {
        sample.layer_.set_tangent_frame(rs.t, rs.b, rs.n);
    }

    sample.layer_.set(refraction_color_, absorption_coefficient_, ior_, rs.ior, thickness_);

    return sample;
}

float3 Glass_thin::thin_absorption(f_float3 wo, f_float3 n, float2 uv, float time,
                                   Sampler_filter filter, Worker const& worker) const {
    float const n_dot_wi              = clamp_abs_dot(wo, n);
    float const approximated_distance = thickness_ / n_dot_wi;

    float3 const attenuation = rendering::attenuation(approximated_distance,
                                                      absorption_coefficient_);

    return opacity(uv, time, filter, worker) * (1.f - refraction_color_ * attenuation);
}

float Glass_thin::ior() const {
    return ior_;
}

bool Glass_thin::has_tinted_shadow() const {
    return true;
}

bool Glass_thin::is_scattering_volume() const {
    return false;
}

size_t Glass_thin::num_bytes() const {
    return sizeof(*this);
}

void Glass_thin::set_normal_map(Texture_adapter const& normal_map) {
    normal_map_ = normal_map;
}

void Glass_thin::set_refraction_color(float3 const& color) {
    refraction_color_ = color;
}

void Glass_thin::set_attenuation(float3 const& absorption_color, float distance) {
    absorption_coefficient_ = extinction_coefficient(absorption_color, distance);
}
void Glass_thin::set_ior(float ior) {
    ior_ = ior;
}

void Glass_thin::set_thickness(float thickness) {
    thickness_ = thickness;
}

size_t Glass_thin::sample_size() {
    return sizeof(Sample_thin);
}

}  // namespace scene::material::glass
