#pragma once

#include "scene/material/material.hpp"

namespace scene::material::glass {

class Glass_thin : public Material {
  public:
    Glass_thin(Sampler_settings const& sampler_settings);

    virtual material::Sample const& sample(float3 const& wo, Renderstate const& rs,
                                           Sampler_filter filter, sampler::Sampler& sampler,
                                           Worker const& worker,
                                           uint32_t      depth) const override final;

    virtual float3 thin_absorption(float3 const& wo, float3 const& n, float2 uv, float time,
                                   Sampler_filter filter,
                                   Worker const&  worker) const override final;

    virtual float ior() const override final;

    virtual bool has_tinted_shadow() const override final;

    virtual bool is_scattering_volume() const final override;

    virtual size_t num_bytes() const override final;

    void set_normal_map(Texture_adapter const& normal_map);

    void set_refraction_color(float3 const& color);
    void set_attenuation(float3 const& absorption_color, float distance);
    void set_ior(float ior);
    void set_thickness(float thickness);

    static size_t sample_size();

  private:
    Texture_adapter normal_map_;

    float3 refraction_color_;
    float3 absorption_coefficient_;
    float  ior_;
    float  thickness_;
};

}  // namespace scene::material::glass
