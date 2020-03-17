#include "substitute_coating_subsurface_material.hpp"
#include "base/math/matrix4x4.inl"
#include "base/math/ray.inl"
#include "base/math/vector4.inl"
#include "base/spectrum/mapping.hpp"
#include "scene/entity/composed_transformation.hpp"
//#include "scene/material/coating/coating.inl"
#include "scene/material/collision_coefficients.inl"
#include "scene/material/volumetric/volumetric_octree_builder.hpp"
#include "scene/material/volumetric/volumetric_sample.hpp"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.inl"
#include "substitute_coating_material.inl"
#include "substitute_coating_sample.inl"
#include "substitute_coating_subsurface_sample.hpp"

#include "scene/material/null/null_sample.hpp"

namespace scene::material::substitute {

Material_coating_subsurface::Material_coating_subsurface(Sampler_settings const& sampler_settings)
    : Material_clearcoat(sampler_settings, false) {}

void Material_coating_subsurface::compile(thread::Pool& threads, Scene const& scene) {
    if (density_map_.is_valid()) {
        auto const& texture = density_map_.texture(scene);

        volumetric::Octree_builder builder;
        builder.build(tree_, texture, cm_, threads);
    }

    is_scattering_ = color_map_.is_valid() || any_greater_zero(cc_.s);
}

material::Sample const& Material_coating_subsurface::sample(float3 const& wo, Ray const& /*ray*/,
                                                            Renderstate const& rs, Filter filter,
                                                            Sampler& /*sampler*/,
                                                            Worker& worker) const {
    auto& sampler = worker.sampler_2D(sampler_key(), filter);

    float thickness;
    float weight;
    if (coating_thickness_map_.is_valid()) {
        float const relative_thickness = coating_thickness_map_.sample_1(worker, sampler, rs.uv);

        thickness = coating_.thickness * relative_thickness;
        weight    = relative_thickness > 0.1f ? 1.f : relative_thickness;
    } else {
        thickness = coating_.thickness;
        weight    = 1.f;
    }

    float const coating_ior = lerp(rs.ior, coating_.ior, weight);

    if (rs.subsurface) {
        auto& sample = worker.sample<Sample_coating_subsurface_volumetric>();

        sample.set_basis(rs.geo_n, wo);

        sample.set(anisotropy_);

        set_coating_basis(wo, rs, sampler, worker, sample);

        sample.coating_.set(coating_.absorption_coefficient, thickness, coating_ior,
                            fresnel::schlick_f0(coating_ior, rs.ior), coating_.alpha, weight);

        return sample;
    }

    auto& sample = worker.sample<Sample_coating_subsurface>();

    set_sample(wo, rs, coating_ior, sampler, worker, sample);

    set_coating_basis(wo, rs, sampler, worker, sample);

    sample.coating_.set(coating_.absorption_coefficient, thickness, coating_ior,
                        fresnel::schlick_f0(coating_ior, rs.ior), coating_.alpha, weight);

    sample.set_volumetric(ior_, rs.ior);

    return sample;
}

size_t Material_coating_subsurface::num_bytes() const {
    return sizeof(*this);
}

void Material_coating_subsurface::set_density_map(Texture_adapter const& density_map) {
    density_map_ = density_map;
}

void Material_coating_subsurface::set_attenuation(float3 const& absorption_color,
                                                  float3 const& scattering_color, float distance) {
    if (any_greater_zero(scattering_color)) {
        cc_ = attenuation(absorption_color, scattering_color, distance);
    } else {
        cc_ = {attenuation_coefficient(absorption_color, distance), float3(0.f)};
    }

    cm_ = CM(cc_);

    attenuation_distance_ = distance;
}

void Material_coating_subsurface::set_volumetric_anisotropy(float anisotropy) {
    anisotropy_ = std::clamp(anisotropy, -0.999f, 0.999f);
}

float3 Material_coating_subsurface::absorption_coefficient(float2 uv, Filter filter,
                                                           Worker const& worker) const {
    if (color_map_.is_valid()) {
        auto const&  sampler = worker.sampler_2D(sampler_key(), filter);
        float3 const color   = color_map_.sample_3(worker, sampler, uv);

        return attenuation_coefficient(color, attenuation_distance_);
    }

    return cc_.a;
}

CC Material_coating_subsurface::collision_coefficients() const {
    return cc_;
}

CC Material_coating_subsurface::collision_coefficients(float2 uv, Filter filter,
                                                       Worker const& worker) const {
    SOFT_ASSERT(color_map_.is_valid());

    auto const& sampler = worker.sampler_2D(sampler_key(), filter);

    float3 const color = color_map_.sample_3(worker, sampler, uv);

    return scattering(cc_.a, color);
}

CC Material_coating_subsurface::collision_coefficients(float3 const& p, Filter filter,
                                                       Worker const& worker) const {
    SOFT_ASSERT(density_map_.is_valid());

    float const d = density(p, filter, worker);

    return {d * cc_.a, d * cc_.s};

    //	float3 p_g = 0.5f * (float3(1.f) + p);

    //	float const x = 1.f - (p_g[1] - 0.2f);
    //	float const d = std::clamp(x * x, 0.01f, 1.f);

    //	float3 const c = color(p, filter, worker);

    //	float3 mu_a, mu_s;
    //	attenuation(c, attenuation_distance_, mu_a, mu_s);

    //	return {d * mu_a, d * mu_s};
}

CM Material_coating_subsurface::control_medium() const {
    return cm_;
}

volumetric::Gridtree const* Material_coating_subsurface::volume_tree() const {
    //	return nullptr;
    return &tree_;
}

bool Material_coating_subsurface::is_heterogeneous_volume() const {
    return density_map_.is_valid();
}

bool Material_coating_subsurface::is_textured_volume() const {
    return color_map_.is_valid();
}

bool Material_coating_subsurface::is_scattering_volume() const {
    return is_scattering_;
}

bool Material_coating_subsurface::is_caustic() const {
    return true;
}

size_t Material_coating_subsurface::sample_size() {
    return sizeof(Sample_coating_subsurface);
}

float Material_coating_subsurface::density(float3 const& p, Filter filter,
                                           Worker const& worker) const {
    // p is in object space already

    float3 const p_g = 0.5f * (float3(1.f) + p);

    auto const& sampler = worker.sampler_3D(sampler_key(), filter);

    return density_map_.sample_1(worker, sampler, p_g);
}

float3 Material_coating_subsurface::color(float3 const& p, Filter /*filter*/,
                                          Worker const& /*worker*/) const {
    float3 const p_g = 0.5f * (float3(1.f) + p);

    //	auto const& sampler = worker.sampler_3D(sampler_key(), filter);

    //	float const d = std::min(16.f * density_map_.sample_1(worker, sampler, p_g), 1.f);

    float const x = 1.f - (p_g[1] - 0.5f);
    float const d = std::clamp(x * x, 0.1f, 1.f);

    //	return float3(d);

    return math::max(d * spectrum::heatmap(p_g[0] + 0.15f), 0.25f);
}

}  // namespace scene::material::substitute
