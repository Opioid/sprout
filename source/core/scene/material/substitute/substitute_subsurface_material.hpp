#ifndef SU_CORE_SCENE_MATERIAL_SUBSTITUTE_SUBSURFACE_MATERIAL_HPP
#define SU_CORE_SCENE_MATERIAL_SUBSTITUTE_SUBSURFACE_MATERIAL_HPP

#include "scene/material/volumetric/volumetric_octree.hpp"
#include "substitute_base_material.hpp"

namespace scene::material::substitute {

class Material_subsurface final : public Material_base {
  public:
    Material_subsurface(Sampler_settings const& sampler_settings);

    void commit(Threads& threads, Scene const& scene) final;

    material::Sample const& sample(float3 const& wo, Ray const& ray, Renderstate const& rs,
                                   Filter filter, Sampler& sampler, Worker& worker) const final;

    void set_density_map(Texture_adapter const& density_map);

    void set_attenuation(float3 const& absorption_color, float3 const& scattering_color,
                         float distance);

    CC collision_coefficients(float2 uv, Filter filter, Worker const& worker) const final;

    CC collision_coefficients(float3 const& p, Filter filter, Worker const& worker) const final;

    volumetric::Gridtree const* volume_tree() const final;

    static size_t sample_size();

  private:
    float density(float3 const& p, Filter filter, Worker const& worker) const;

    float3 color(float3 const& p, Filter filter, Worker const& worker) const;

    Texture_adapter density_map_;

    volumetric::Gridtree tree_;
};

}  // namespace scene::material::substitute

#endif
