#ifndef SU_CORE_SCENE_MATERIAL_SUBSTITUTE_SUBSURFACE_MATERIAL_HPP
#define SU_CORE_SCENE_MATERIAL_SUBSTITUTE_SUBSURFACE_MATERIAL_HPP

#include "scene/material/volumetric/volumetric_octree.hpp"
#include "substitute_base_material.hpp"
#include "substitute_subsurface_sample.hpp"

namespace scene::material::substitute {

class Material_subsurface final : public Material_base {
  public:
    Material_subsurface(Sampler_settings const& sampler_settings) noexcept;

    void compile() noexcept override final;

    material::Sample const& sample(float3 const& wo, Renderstate const& rs, Sampler_filter filter,
                                   sampler::Sampler& sampler, Worker const& worker,
                                   uint32_t depth) const noexcept override final;

    size_t num_bytes() const noexcept override final;

    void set_density_map(Texture_adapter const& density_map) noexcept;

    void set_attenuation(float3 const& absorption_color, float3 const& scattering_color,
                         float distance) noexcept;

    void set_volumetric_anisotropy(float anisotropy) noexcept;

    float3 emission(math::Ray const& ray, Transformation const& transformation, float step_size,
                    rnd::Generator& rng, Sampler_filter filter, Worker const& worker) const
        noexcept override final;

    float3 absorption_coefficient(float2 uv, Sampler_filter filter, Worker const& worker) const
        noexcept override final;

    CC collision_coefficients() const noexcept override final;

    CC collision_coefficients(float2 uv, Sampler_filter filter, Worker const& worker) const
        noexcept override final;

    CC collision_coefficients(float3 const& p, Sampler_filter filter, Worker const& worker) const
        noexcept override final;

    CM control_medium() const noexcept override final;

    volumetric::Gridtree const* volume_tree() const noexcept override final;

    bool is_heterogeneous_volume() const noexcept override final;
    bool is_textured_volume() const noexcept override final;

    static size_t sample_size() noexcept;

  private:
    float density(float3 const& p, Sampler_filter filter, Worker const& worker) const noexcept;

    float3 color(float3 const& p, Sampler_filter filter, Worker const& worker) const noexcept;

    Texture_adapter density_map_;

    CC cc_;
    CM cm_;

    float anisotropy_;
    float attenuation_distance_;

    volumetric::Gridtree tree_;
};

}  // namespace scene::material::substitute

#endif
