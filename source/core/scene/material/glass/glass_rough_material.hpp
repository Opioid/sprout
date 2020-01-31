#ifndef SU_CORE_SCENE_MATERIAL_GLASS_ROUGH_MATERIAL_HPP
#define SU_CORE_SCENE_MATERIAL_GLASS_ROUGH_MATERIAL_HPP

#include "scene/material/material.hpp"

namespace scene::material::glass {

class Glass_rough : public Material {
  public:
    Glass_rough(Sampler_settings const& sampler_settings) noexcept;

    material::Sample const& sample(float3 const& wo, Ray const& ray, Renderstate const& rs,
                                   Filter filter, Sampler& sampler, Worker const& worker) const
        noexcept final;

    float3 absorption_coefficient(float2 uv, Filter filter, Worker const& worker) const
        noexcept final;

    float ior() const noexcept final;

    size_t num_bytes() const noexcept final;

    void set_normal_map(Texture_adapter const& normal_map) noexcept;
    void set_roughness_map(Texture_adapter const& roughness_map) noexcept;

    void set_refraction_color(float3 const& color) noexcept;
    void set_attenuation(float3 const& absorption_color, float distance) noexcept;
    void set_ior(float ior) noexcept;
    void set_roughness(float roughness) noexcept;

    bool is_caustic() const noexcept final;

    static size_t sample_size() noexcept;

  protected:
    Texture_adapter normal_map_;
    Texture_adapter roughness_map_;

    float3 refraction_color_;
    float3 absorption_coefficient_;

    float attenuation_distance_;
    float ior_;
    float alpha_;
};

}  // namespace scene::material::glass

#endif
