#ifndef SU_CORE_SCENE_MATERIAL_SUBSTITUTE_BASE_MATERIAL_HPP
#define SU_CORE_SCENE_MATERIAL_SUBSTITUTE_BASE_MATERIAL_HPP

#include "scene/material/material.hpp"

namespace image::texture {
class Sampler_2D;
}

namespace scene::material::substitute {

class Material_base : public material::Material {
  public:
    Material_base(Sampler_settings sampler_settings, bool two_sided);

    void commit(Threads& threads, Scene const& scene) override;

    float3 evaluate_radiance(float3_p wi, float3_p uvw, float extent, Filter filter,
                             Worker const& worker) const override;

    float3 average_radiance(float area) const final;

    void set_normal_map(Turbotexture const& normal_map);
    void set_surface_map(Turbotexture const& surface_map);
    void set_emission_map(Turbotexture const& emission_map);

    void set_color(float3_p color);

    void set_roughness(float roughness);
    void set_metallic(float metallic);
    void set_emission_factor(float emission_factor);

  protected:
    using Texture_sampler_2D = image::texture::Sampler_2D;

    template <typename Sample>
    void set_sample(float3_p wo, Renderstate const& rs, float ior_outside,
                    Texture_sampler_2D const& sampler, Worker const& worker, Sample& sample) const;

    Turbotexture normal_map_;
    Turbotexture surface_map_;
    Turbotexture emission_map_;

    float3 color_;
    float3 average_emission_;

    float alpha_;
    float metallic_;
    float emission_factor_;
};

}  // namespace scene::material::substitute

#endif
