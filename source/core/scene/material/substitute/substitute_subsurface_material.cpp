#include "substitute_subsurface_material.hpp"
#include "base/math/matrix4x4.inl"
#include "base/math/ray.inl"
#include "base/math/vector4.inl"
#include "base/spectrum/mapping.hpp"
#include "scene/entity/composed_transformation.hpp"
#include "scene/material/collision_coefficients.inl"
#include "scene/material/material.inl"
#include "scene/material/volumetric/volumetric_octree_builder.hpp"
#include "scene/material/volumetric/volumetric_sample.hpp"
#include "scene/scene_renderstate.hpp"
#include "scene/scene_worker.inl"
#include "substitute_base_material.inl"
#include "substitute_base_sample.inl"
#include "substitute_subsurface_sample.hpp"

#include "scene/material/null/null_sample.hpp"

namespace scene::material::substitute {

Material_subsurface::Material_subsurface(Sampler_settings const& sampler_settings)
    : Material_base(sampler_settings, false) {
    properties_.set(Property::Caustic);
}

void Material_subsurface::commit(Threads& threads, Scene const& scene) {
    if (density_map_.is_valid()) {
        auto const& texture = density_map_.texture(scene);

        volumetric::Octree_builder builder;
        builder.build(tree_, texture, &cc_, threads);
    }

    properties_.set(Property::Scattering_volume, color_map_.is_valid() || any_greater_zero(cc_.s));
    properties_.set(Property::Textured_volume, color_map_.is_valid());
    properties_.set(Property::Heterogeneous_volume, density_map_.is_valid());
}

material::Sample const& Material_subsurface::sample(float3_p           wo, Ray const& /*ray*/,
                                                    Renderstate const& rs, Filter filter,
                                                    Sampler& /*sampler*/, Worker& worker) const {
    if (rs.subsurface) {
        auto& sample = worker.sample<volumetric::Sample>();

        sample.set_common(rs, wo, float3(0.f), float3(0.f), rs.alpha);

        sample.set(volumetric_anisotropy_);

        return sample;
    }

    auto& sample = worker.sample<Sample_subsurface>();

    auto& sampler = worker.sampler_2D(sampler_key(), filter);

    set_sample(wo, rs, rs.ior, sampler, worker, sample);

    sample.set_volumetric(ior_, rs.ior);

    return sample;
}

void Material_subsurface::set_density_map(Texture_adapter const& density_map) {
    density_map_ = density_map;
}

CC Material_subsurface::collision_coefficients(float3_p p, Filter filter,
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

volumetric::Gridtree const* Material_subsurface::volume_tree() const {
    //	return nullptr;
    return &tree_;
}

size_t Material_subsurface::sample_size() {
    return sizeof(Sample_subsurface);
}

float Material_subsurface::density(float3_p p, Filter filter, Worker const& worker) const {
    // p is in object space already

    float3 const p_g = 0.5f * (float3(1.f) + p);

    auto const& sampler = worker.sampler_3D(sampler_key(), filter);

    return density_map_.sample_1(worker, sampler, p_g);
}

float3 Material_subsurface::color(float3_p p, Filter /*filter*/, Worker const& /*worker*/) const {
    float3 const p_g = 0.5f * (float3(1.f) + p);

    //	auto const& sampler = worker.sampler_3D(sampler_key(), filter);

    //	float const d = std::min(16.f * density_map_.sample_1(worker, sampler, p_g), 1.f);

    float const x = 1.f - (p_g[1] - 0.5f);
    float const d = std::clamp(x * x, 0.1f, 1.f);

    //	return float3(d);

    return math::max(d * spectrum::heatmap(p_g[0] + 0.15f), 0.25f);
}

}  // namespace scene::material::substitute
