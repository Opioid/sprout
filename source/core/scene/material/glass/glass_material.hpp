#ifndef SU_CORE_SCENE_MATERIAL_GLASS_MATERIAL_HPP
#define SU_CORE_SCENE_MATERIAL_GLASS_MATERIAL_HPP

#include "scene/material/material.hpp"

namespace scene::material::glass {

class Glass : public Material {
  public:
    Glass(Sampler_settings const& sampler_settings) noexcept;

    material::Sample const& sample(float3 const& wo, Renderstate const& rs, Filter filter,
                                   sampler::Sampler& sampler, Worker const& worker,
                                   uint32_t depth) const noexcept override;

    float3 absorption_coefficient(float2 uv, Filter filter, Worker const& worker) const
        noexcept override final;

    float ior() const noexcept override final;

    bool is_scattering_volume() const noexcept override final;

    size_t num_bytes() const noexcept override;

    void set_normal_map(Texture_adapter const& normal_map) noexcept;

    void set_refraction_color(float3 const& color) noexcept;
    void set_attenuation(float3 const& absorption_color, float distance) noexcept;
    void set_ior(float ior) noexcept;

    bool is_caustic() const noexcept override final;

    static size_t sample_size() noexcept;

  protected:
    Texture_adapter normal_map_;

    float3 refraction_color_;
    float3 absorption_coefficient_;
    float3 absorption_color_;
    float  attenuation_distance_;
    float  ior_;
};

}  // namespace scene::material::glass

#endif
