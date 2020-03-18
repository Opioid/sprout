#include "glass_thin_material.hpp"
#include "base/math/vector4.inl"
#include "glass_thin_sample.hpp"
#include "image/texture/texture_adapter.inl"
#include "rendering/integrator/integrator_helper.hpp"
#include "scene/material/collision_coefficients.inl"
#include "scene/material/fresnel/fresnel.inl"
#include "scene/material/material_helper.hpp"
#include "scene/material/material_sample.inl"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.inl"

namespace scene::material::glass {

Glass_thin::Glass_thin(Sampler_settings const& sampler_settings)
    : Material(sampler_settings, true) {
    properties_.set(Property::Tinted_shadow);
}

material::Sample const& Glass_thin::sample(float3 const&      wo, Ray const& /*ray*/,
                                           Renderstate const& rs, Filter filter,
                                           Sampler& /*sampler*/, Worker& worker) const {
    auto& sample = worker.sample<Sample_thin>();

    sample.set_basis(rs.geo_n, wo);

    if (normal_map_.is_valid()) {
        auto const&  sampler = worker.sampler_2D(sampler_key(), filter);
        float3 const n       = sample_normal(wo, rs, normal_map_, sampler, worker);
        sample.layer_.set_tangent_frame(n);
    } else {
        sample.layer_.set_tangent_frame(rs.t, rs.b, rs.n);
    }

    sample.set(refraction_color_, absorption_coefficient_, ior_, rs.ior, thickness_);

    return sample;
}

float3 Glass_thin::thin_absorption(float3 const& wi, float3 const& n, float2 uv, uint64_t time,
                                   Filter filter, Worker const& worker) const {
    float const eta_i = 1.f;
    float const eta_t = ior_;

    float const n_dot_wo = std::min(std::abs(dot(n, wi)), 1.f);
    float const eta      = eta_i / eta_t;
    float const sint2    = (eta * eta) * (1.f - n_dot_wo * n_dot_wo);

    if (sint2 >= 1.f) {
        return float3(0.f);
    }

    float const n_dot_t = std::sqrt(1.f - sint2);

    float const f = fresnel::dielectric(n_dot_wo, n_dot_t, eta_i, eta_t);

    float const n_dot_wi = clamp(n_dot_wo);

    float const approximated_distance = thickness_ / n_dot_wi;

    float3 const attenuation = rendering::attenuation(approximated_distance,
                                                      absorption_coefficient_);

    float const o = opacity(uv, time, filter, worker);

    float3 const ta = min((1.f - o) + (refraction_color_ * attenuation), 1.f);

    return (1.f - f) * ta;
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
    absorption_coefficient_ = attenuation_coefficient(absorption_color, distance);
}

void Glass_thin::set_thickness(float thickness) {
    thickness_ = thickness;
}

size_t Glass_thin::sample_size() {
    return sizeof(Sample_thin);
}

}  // namespace scene::material::glass
