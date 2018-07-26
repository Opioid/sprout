#ifndef SU_CORE_SCENE_MATERIAL_SUBSTITUTE_BASE_MATERIAL_HPP
#define SU_CORE_SCENE_MATERIAL_SUBSTITUTE_BASE_MATERIAL_HPP

#include "scene/material/material.hpp"

namespace scene::material::substitute {

class Material_base : public material::Material {
  public:
    Material_base(Sampler_settings const& sampler_settings, bool two_sided);

    virtual float3 evaluate_radiance(f_float3 wi, float2 uv, float area, float time,
                                     Sampler_filter filter,
                                     Worker const&  worker) const override final;

    virtual float3 average_radiance(float area) const override final;

    virtual bool has_emission_map() const override final;

    virtual float ior() const override final;

    virtual void set_ior(float ior, float external_ior = 1.f);

    virtual bool is_caustic() const override;

    void set_color_map(Texture_adapter const& color_map);
    void set_normal_map(Texture_adapter const& normal_map);
    void set_surface_map(Texture_adapter const& surface_map);
    void set_emission_map(Texture_adapter const& emission_map);

    void set_color(f_float3 color);
    void set_roughness(float roughness);
    void set_metallic(float metallic);
    void set_emission_factor(float emission_factor);

  protected:
    using Texture_sampler_2D = image::texture::sampler::Sampler_2D;

    template <typename Sample>
    void set_sample(f_float3 wo, Renderstate const& rs, Texture_sampler_2D const& sampler,
                    Sample& sample) const;

    Texture_adapter color_map_;
    Texture_adapter normal_map_;
    Texture_adapter surface_map_;
    Texture_adapter emission_map_;

    float3 color_;

    float  ior_;
    float  constant_f0_;
    float  roughness_;
    float  metallic_;
    float  emission_factor_;
};

}  // namespace scene::material::substitute

#endif
