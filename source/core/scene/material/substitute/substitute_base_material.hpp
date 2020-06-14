#ifndef SU_CORE_SCENE_MATERIAL_SUBSTITUTE_BASE_MATERIAL_HPP
#define SU_CORE_SCENE_MATERIAL_SUBSTITUTE_BASE_MATERIAL_HPP

#include "scene/material/material.hpp"

namespace scene::material::substitute {

class Material_base : public material::Material {
  public:
    Material_base(Sampler_settings const& sampler_settings, bool two_sided);

    void commit(thread::Pool& threads, Scene const& scene) override;

    float3 evaluate_radiance(float3 const& wi, float3 const& uvw, float extent, Filter filter,
                             Worker const& worker) const override;

    float3 average_radiance(float area, Scene const& scene) const final;

    void set_color_map(Texture_adapter const& color_map);
    void set_normal_map(Texture_adapter const& normal_map);
    void set_surface_map(Texture_adapter const& surface_map);
    void set_emission_map(Texture_adapter const& emission_map);

    void set_color(float3 const& color);

    void set_roughness(float roughness);
    void set_metallic(float metallic);
    void set_emission_factor(float emission_factor);

  protected:
    using Texture_sampler_2D = image::texture::Sampler_2D;

    template <typename Sample>
    void set_sample(float3 const& wo, Ray const& ray, Renderstate const& rs, float ior_outside,
                    Texture_sampler_2D const& sampler, Worker const& worker, Sample& sample) const;

    Texture_adapter color_map_;
    Texture_adapter normal_map_;
    Texture_adapter surface_map_;
    Texture_adapter emission_map_;

    float3 color_;

    float alpha_;
    float metallic_;
    float emission_factor_;
};

}  // namespace scene::material::substitute

#endif
