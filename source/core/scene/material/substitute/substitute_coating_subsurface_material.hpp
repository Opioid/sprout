#ifndef SU_CORE_SCENE_MATERIAL_SUBSTITUTE_COATING_SUBSURFACE_MATERIAL_HPP
#define SU_CORE_SCENE_MATERIAL_SUBSTITUTE_COATING_SUBSURFACE_MATERIAL_HPP

#include "scene/material/volumetric/volumetric_octree.hpp"
#include "substitute_coating_material.hpp"

namespace scene::material::substitute {

class Material_coating_subsurface final : public Material_clearcoat {
  public:
    Material_coating_subsurface(Sampler_settings const& sampler_settings);

    void compile(thread::Pool& threads, Scene const& scene) final;

    material::Sample const& sample(float3 const& wo, Ray const& ray, Renderstate const& rs,
                                   Filter filter, Sampler& sampler, Worker& worker) const final;

    size_t num_bytes() const final;

    void set_density_map(Texture_adapter const& density_map);

    void set_attenuation(float3 const& absorption_color, float3 const& scattering_color,
                         float distance);

    void set_volumetric_anisotropy(float anisotropy);

    float3 absorption_coefficient(float2 uv, Filter filter, Worker const& worker) const final;

    CC collision_coefficients() const final;

    CC collision_coefficients(float2 uv, Filter filter, Worker const& worker) const final;

    CC collision_coefficients(float3 const& p, Filter filter, Worker const& worker) const final;

    CM control_medium() const final;

    volumetric::Gridtree const* volume_tree() const final;

    bool is_heterogeneous_volume() const final;
    bool is_textured_volume() const final;
    bool is_scattering_volume() const final;

    bool is_caustic() const final;

    static size_t sample_size();

  private:
    float density(float3 const& p, Filter filter, Worker const& worker) const;

    float3 color(float3 const& p, Filter filter, Worker const& worker) const;

    Texture_adapter density_map_;

    CC cc_;
    CM cm_;

    float anisotropy_;
    float attenuation_distance_;

    volumetric::Gridtree tree_;

    bool is_scattering_;
};

}  // namespace scene::material::substitute

#endif
