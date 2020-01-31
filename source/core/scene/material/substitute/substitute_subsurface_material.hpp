#ifndef SU_CORE_SCENE_MATERIAL_SUBSTITUTE_SUBSURFACE_MATERIAL_HPP
#define SU_CORE_SCENE_MATERIAL_SUBSTITUTE_SUBSURFACE_MATERIAL_HPP

#include "scene/material/volumetric/volumetric_octree.hpp"
#include "substitute_base_material.hpp"

namespace scene::material::substitute {

class Material_subsurface final : public Material_base {
  public:
    Material_subsurface(Sampler_settings const& sampler_settings) noexcept;

    void compile(thread::Pool& threads, Scene const& scene) noexcept final;

    material::Sample const& sample(float3 const& wo, Ray const& ray, Renderstate const& rs,
                                   Filter filter, Sampler& sampler, Worker const& worker) const
        noexcept final;

    float volume_border_hack(float3 const& wi, float3 const& n, Worker const& worker) const
        noexcept final;

    size_t num_bytes() const noexcept final;

    void set_density_map(Texture_adapter const& density_map) noexcept;

    void set_attenuation(float3 const& absorption_color, float3 const& scattering_color,
                         float distance) noexcept;

    void set_volumetric_anisotropy(float anisotropy) noexcept;

    float3 absorption_coefficient(float2 uv, Filter filter, Worker const& worker) const
        noexcept final;

    CC collision_coefficients() const noexcept final;

    CC collision_coefficients(float2 uv, Filter filter, Worker const& worker) const noexcept final;

    CC collision_coefficients(float3 const& p, Filter filter, Worker const& worker) const
        noexcept final;

    CM control_medium() const noexcept final;

    volumetric::Gridtree const* volume_tree() const noexcept final;

    bool is_heterogeneous_volume() const noexcept final;
    bool is_textured_volume() const noexcept final;
    bool is_scattering_volume() const noexcept final;

    bool is_caustic() const noexcept final;

    static size_t sample_size() noexcept;

  private:
    float density(float3 const& p, Filter filter, Worker const& worker) const noexcept;

    float3 color(float3 const& p, Filter filter, Worker const& worker) const noexcept;

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
