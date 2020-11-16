#pragma once

#include "scene/material/coating/coating.hpp"
#include "scene/material/material.hpp"

namespace scene::material::metallic_paint {

class Material : public material::Material {
  public:
    Material(Sampler_settings const& sampler_settings, bool two_sided);

    material::Sample const& sample(float3_p wo, Ray const& ray, Renderstate const& rs,
                                   Filter filter, Sampler& sampler, Worker& worker) const final;

    void set_color(float3_p a, float3_p b);
    void set_roughness(float roughness);

    void set_flakes_mask(Texture_adapter const& mask);
    void set_flakes_normal_map(Texture_adapter const& normal_map);
    void set_flakes_ior(float3_p ior);
    void set_flakes_absorption(float3_p absorption);
    void set_flakes_roughness(float roughness);

    void set_coating_thickness(float thickness);
    void set_coating_attenuation(float3_p absorption_color, float distance);
    void set_coating_ior(float ior);
    void set_coating_roughness(float roughness);

    static size_t sample_size();

  protected:
    Texture_adapter flakes_mask_;
    Texture_adapter flakes_normal_map_;

    float3 color_a_;
    float3 color_b_;

    float3 flakes_ior_;
    float3 flakes_absorption_;

    float alpha_;

    float flakes_alpha_;

    struct Clearcoat_data {
        float3 absorption_coef;

        float thickness;
        float ior;
        float alpha;
    };

    Clearcoat_data coating_;
};

}  // namespace scene::material::metallic_paint
