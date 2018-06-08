#pragma once

#include "scene/material/material.hpp"

namespace scene::material::metal {

class Material_isotropic : public Material {
  public:
    Material_isotropic(Sampler_settings const& sampler_settings, bool two_sided);

    virtual const material::Sample& sample(f_float3 wo, Renderstate const& rs,
                                           Sampler_filter filter, sampler::Sampler& sampler,
                                           Worker const& worker) const override final;

    virtual float ior() const override final;

    virtual size_t num_bytes() const override final;

    void set_normal_map(Texture_adapter const& normal_map);

    void set_ior(float3 const& ior);
    void set_absorption(float3 const& absorption);
    void set_roughness(float roughness);

    static size_t sample_size();

  protected:
    Texture_adapter normal_map_;

    float3 ior_;
    float3 absorption_;

    float roughness_;
};

class Material_anisotropic : public Material {
  public:
    Material_anisotropic(Sampler_settings const& sampler_settings, bool two_sided);

    virtual const material::Sample& sample(f_float3 wo, Renderstate const& rs,
                                           Sampler_filter filter, sampler::Sampler& sampler,
                                           Worker const& worker) const override final;

    virtual float ior() const override final;

    virtual size_t num_bytes() const override final;

    void set_normal_map(Texture_adapter const& normal_map);
    void set_direction_map(Texture_adapter const& direction_map);

    void set_ior(float3 const& ior);
    void set_absorption(float3 const& absorption);
    void set_roughness(float2 roughness);

    static size_t sample_size();

  protected:
    Texture_adapter normal_map_;
    Texture_adapter direction_map_;

    float3 ior_;
    float3 absorption_;

    float2 roughness_;
};

}  // namespace scene::material::metal
