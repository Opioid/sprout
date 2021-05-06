#include "glass_thin_material.hpp"
#include "base/math/vector3.inl"
#include "glass_thin_sample.hpp"

#include "rendering/integrator/integrator_helper.hpp"
#include "scene/material/collision_coefficients.inl"
#include "scene/material/fresnel/fresnel.inl"
#include "scene/material/material.inl"
#include "scene/material/material_helper.hpp"
#include "scene/material/material_sample.inl"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.inl"

namespace scene::material::glass {

Glass_thin::Glass_thin(Sampler_settings sampler_settings) : Material(sampler_settings, true) {
    properties_.set(Property::Caustic);
    properties_.set(Property::Tinted_shadow);
}

material::Sample const& Glass_thin::sample(float3_p wo, Renderstate const& rs, Sampler& /*sampler*/,
                                           Worker& worker) const {
    auto& sample = worker.sample<Sample_thin>();

    if (normal_map_.is_valid()) {
        auto const&  sampler = worker.sampler_2D(sampler_key(), rs.filter);
        float3 const n       = sample_normal(wo, rs, normal_map_, sampler, worker.scene());
        sample.layer_.set_tangent_frame(n);
    } else {
        sample.layer_.set_tangent_frame(rs.t, rs.b, rs.n);
    }

    sample.set_common(rs, wo, refraction_color_, float3(0.f), rs.alpha());
    sample.set(cc_.a, ior_, rs.ior(), thickness_);

    return sample;
}

float3 Glass_thin::thin_absorption(float3_p wi, float3_p n, float2 uv, Filter filter,
                                   Worker const& worker) const {
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

    float const approx_distance = thickness_ / n_dot_wi;

    float3 const attenuation = rendering::attenuation(approx_distance, cc_.a);

    float const o = opacity(uv, filter, worker);

    float3 const ta = min((1.f - o) + (refraction_color_ * attenuation), 1.f);

    return (1.f - f) * ta;
}

void Glass_thin::set_normal_map(Texture const& normal_map) {
    normal_map_ = normal_map;
}

void Glass_thin::set_refraction_color(float3_p color) {
    refraction_color_ = color;
}

void Glass_thin::set_thickness(float thickness) {
    thickness_ = thickness;
}

size_t Glass_thin::sample_size() {
    return sizeof(Sample_thin);
}

}  // namespace scene::material::glass
