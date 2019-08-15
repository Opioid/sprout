#ifndef SU_CORE_SCENE_MATERIAL_SUBSTITUTE_BASE_MATERIAL_HPP
#define SU_CORE_SCENE_MATERIAL_SUBSTITUTE_BASE_MATERIAL_HPP

#include "scene/material/material.hpp"

namespace scene::material::substitute {

class Material_base : public material::Material {
  public:
    Material_base(Sampler_settings const& sampler_settings, bool two_sided) noexcept;

    float3 evaluate_radiance(float3 const& wi, float2 uv, float area, Filter filter,
                             Worker const& worker) const noexcept override;

    float3 average_radiance(float area) const noexcept override final;

    bool has_emission_map() const noexcept override final;

    float ior() const noexcept override final;

    void set_ior(float ior) noexcept;

    bool is_caustic() const noexcept override;

    void set_color_map(Texture_adapter const& color_map) noexcept;
    void set_normal_map(Texture_adapter const& normal_map) noexcept;
    void set_surface_map(Texture_adapter const& surface_map) noexcept;
    void set_emission_map(Texture_adapter const& emission_map) noexcept;

    void set_color(float3 const& color) noexcept;

    void set_roughness(float roughness) noexcept;
    void set_metallic(float metallic) noexcept;
    void set_emission_factor(float emission_factor) noexcept;

  protected:
    using Texture_sampler_2D = image::texture::sampler::Sampler_2D;

    template <typename Sample>
    void set_sample(float3 const& wo, Renderstate const& rs, float ior_outside,
                    Texture_sampler_2D const& sampler, Sample& sample) const noexcept;

    Texture_adapter color_map_;
    Texture_adapter normal_map_;
    Texture_adapter surface_map_;
    Texture_adapter emission_map_;

    float3 color_;

    float ior_;
    float alpha_;
    float metallic_;
    float emission_factor_;
};

}  // namespace scene::material::substitute

#endif
