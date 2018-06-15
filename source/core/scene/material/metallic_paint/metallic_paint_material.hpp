#pragma once

#include "scene/material/coating/coating.hpp"
#include "scene/material/material.hpp"

namespace scene::material::metallic_paint {

class Material : public material::Material {
  public:
    Material(Sampler_settings const& sampler_settings, bool two_sided);

    virtual const material::Sample& sample(f_float3 wo, Renderstate const& rs,
                                           Sampler_filter filter, sampler::Sampler& sampler,
                                           Worker const& worker,
                                           uint32_t      depth) const override final;

    virtual float ior() const override final;

    virtual size_t num_bytes() const override final;

    void set_color(float3 const& a, float3 const& b);
    void set_roughness(float roughness);

    void set_flakes_mask(Texture_adapter const& mask);
    void set_flakes_normal_map(Texture_adapter const& normal_map);
    void set_flakes_ior(float3 const& ior);
    void set_flakes_absorption(float3 const& absorption);
    void set_flakes_roughness(float roughness);

    void set_coating_weight(float weight);
    void set_coating_color(float3 const& color);

    void set_clearcoat(float ior, float roughness);

    static size_t sample_size();

  protected:
    Texture_adapter flakes_mask_;
    Texture_adapter flakes_normal_map_;

    float3 color_a_;
    float3 color_b_;

    float alpha_;
    float alpha2_;

    float3 flakes_ior_;
    float3 flakes_absorption_;
    float  flakes_alpha_;
    float  flakes_alpha2_;

    float ior_;

    coating::Clearcoat coating_;
};

}  // namespace scene::material::metallic_paint
