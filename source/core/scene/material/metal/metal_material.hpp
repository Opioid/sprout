#pragma once

#include "scene/material/material.hpp"

namespace scene::material::metal {

class Material_isotropic : public Material {
  public:
    Material_isotropic(Sampler_settings const& sampler_settings, bool two_sided) noexcept;

    material::Sample const& sample(float3 const& wo, Ray const& ray, Renderstate const& rs,
                                   Filter filter, sampler::Sampler& sampler,
                                   Worker const& worker) const noexcept override final;

    float ior() const noexcept override final;

    size_t num_bytes() const noexcept override final;

    void set_normal_map(Texture_adapter const& normal_map) noexcept;

    void set_ior(float3 const& ior) noexcept;
    void set_absorption(float3 const& absorption) noexcept;
    void set_roughness(float roughness) noexcept;

    static size_t sample_size() noexcept;

  protected:
    Texture_adapter normal_map_;

    float3 ior_;
    float3 absorption_;

    float alpha_;
};

class Material_anisotropic : public Material {
  public:
    Material_anisotropic(Sampler_settings const& sampler_settings, bool two_sided) noexcept;

    material::Sample const& sample(float3 const& wo, Ray const& ray, Renderstate const& rs,
                                   Filter filter, sampler::Sampler& sampler,
                                   Worker const& worker) const noexcept override final;

    float ior() const noexcept override final;

    size_t num_bytes() const noexcept override final;

    void set_normal_map(Texture_adapter const& normal_map) noexcept;
    void set_direction_map(Texture_adapter const& direction_map) noexcept;

    void set_ior(float3 const& ior) noexcept;
    void set_absorption(float3 const& absorption) noexcept;
    void set_roughness(float2 roughness) noexcept;

    static size_t sample_size() noexcept;

  protected:
    Texture_adapter normal_map_;
    Texture_adapter direction_map_;

    float3 ior_;
    float3 absorption_;

    float2 roughness_;
};

}  // namespace scene::material::metal
