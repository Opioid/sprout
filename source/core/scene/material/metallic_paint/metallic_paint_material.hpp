#pragma once

#include "scene/material/coating/coating.hpp"
#include "scene/material/material.hpp"

namespace scene::material::metallic_paint {

class Material : public material::Material {
  public:
    Material(Sampler_settings const& sampler_settings, bool two_sided) noexcept;

    material::Sample const& sample(float3 const& wo, Ray const& ray, Renderstate const& rs,
                                   Filter filter, sampler::Sampler& sampler, Worker const& worker,
                                   uint32_t depth) const noexcept override final;

    float ior() const noexcept override final;

    size_t num_bytes() const noexcept override final;

    void set_color(float3 const& a, float3 const& b) noexcept;
    void set_roughness(float roughness) noexcept;

    void set_flakes_mask(Texture_adapter const& mask) noexcept;
    void set_flakes_normal_map(Texture_adapter const& normal_map) noexcept;
    void set_flakes_ior(float3 const& ior) noexcept;
    void set_flakes_absorption(float3 const& absorption) noexcept;
    void set_flakes_roughness(float roughness) noexcept;

    void set_coating_thickness(float thickness) noexcept;
    void set_coating_attenuation(float3 const& absorption_color, float distance) noexcept;
    void set_coating_ior(float ior) noexcept;
    void set_coating_roughness(float roughness) noexcept;

    static size_t sample_size() noexcept;

  protected:
    Texture_adapter flakes_mask_;
    Texture_adapter flakes_normal_map_;

    float3 color_a_;
    float3 color_b_;

    float3 flakes_ior_;
    float3 flakes_absorption_;

    float alpha_;

    float flakes_alpha_;

    float ior_;

    struct Clearcoat_data {
        float3 absorption_coefficient;

        float thickness;
        float ior;
        float alpha;
    };

    Clearcoat_data coating_;
};

}  // namespace scene::material::metallic_paint
