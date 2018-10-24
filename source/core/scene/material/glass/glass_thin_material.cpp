#include "glass_thin_material.hpp"
#include "base/math/vector4.inl"
#include "glass_thin_sample.hpp"
#include "image/texture/texture_adapter.inl"
#include "rendering/integrator/integrator_helper.hpp"
#include "scene/material/collision_coefficients.inl"
#include "scene/material/material_helper.hpp"
#include "scene/material/material_sample.inl"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.inl"

namespace scene::material::glass {

Glass_thin::Glass_thin(Sampler_settings const& sampler_settings) noexcept
    : Material(sampler_settings, true) {}

material::Sample const& Glass_thin::sample(float3 const& wo, Renderstate const& rs, Filter filter,
                                           sampler::Sampler& /*sampler*/, Worker const& worker,
                                           uint32_t depth) const noexcept {
    auto& sample = worker.sample<Sample_thin>(depth);

    sample.set_basis(rs.geo_n, wo);

    if (normal_map_.is_valid()) {
        auto const&  sampler = worker.sampler_2D(sampler_key(), filter);
        float3 const n       = sample_normal(wo, rs, normal_map_, sampler);
        sample.layer_.set_tangent_frame(n);
    } else {
        sample.layer_.set_tangent_frame(rs.t, rs.b, rs.n);
    }

    sample.set(refraction_color_, absorption_coefficient_, ior_, rs.ior, thickness_);

    return sample;
}

float3 Glass_thin::thin_absorption(float3 const& wo, float3 const& n, float2 uv, uint64_t time,
                                   Filter filter, Worker const& worker) const noexcept {
    float const n_dot_wi              = clamp_abs_dot(wo, n);
    float const approximated_distance = thickness_ / n_dot_wi;

    float3 const attenuation = rendering::attenuation(approximated_distance,
                                                      absorption_coefficient_);

    return opacity(uv, time, filter, worker) * (1.f - refraction_color_ * attenuation);
}

float Glass_thin::ior() const noexcept {
    return ior_;
}

bool Glass_thin::has_tinted_shadow() const noexcept {
    return true;
}

bool Glass_thin::is_scattering_volume() const noexcept {
    return false;
}

size_t Glass_thin::num_bytes() const noexcept {
    return sizeof(*this);
}

void Glass_thin::set_normal_map(Texture_adapter const& normal_map) noexcept {
    normal_map_ = normal_map;
}

void Glass_thin::set_refraction_color(float3 const& color) noexcept {
    refraction_color_ = color;
}

void Glass_thin::set_attenuation(float3 const& absorption_color, float distance) noexcept {
    absorption_coefficient_ = extinction_coefficient(absorption_color, distance);
}
void Glass_thin::set_ior(float ior) noexcept {
    ior_ = ior;
}

void Glass_thin::set_thickness(float thickness) noexcept {
    thickness_ = thickness;
}

size_t Glass_thin::sample_size() noexcept {
    return sizeof(Sample_thin);
}

}  // namespace scene::material::glass
