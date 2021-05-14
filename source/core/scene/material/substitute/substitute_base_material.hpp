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

    float3 evaluate_radiance(float3_p wi, float3_p n, float3_p uvw, float extent, Filter filter,
                             Worker const& worker) const override;

    float3 prepare_sampling(Shape const& shape, uint32_t part, Transformation const& trafo,
                            float area, Scene const& scene, Threads& threads) final;

    void set_normal_map(Texture const& normal_map);
    void set_rotation_map(Texture const& rotation_map);
    void set_surface_map(Texture const& surface_map);
    void set_emission_map(Texture const& emission_map);

    void set_color(float3_p color);

    void set_roughness(float roughness, float anisotropy);
    void set_rotation(float angle);
    void set_metallic(float metallic);
    void set_emission_factor(float emission_factor);

    image::Description useful_texture_description(Scene const& scene) const final;

  protected:
    using Texture_sampler_2D = image::texture::Sampler_2D;

    template <typename Sample>
    void set_sample(float3_p wo, Renderstate const& rs, float ior_outside,
                    Texture_sampler_2D const& sampler, Worker const& worker, Sample& sample) const;

    Texture normal_map_;
    Texture surface_map_;
    Texture rotation_map_;
    Texture emission_map_;

    float3 color_;

    float2 alpha_;

    float anisotropy_;
    float rotation_;
    float metallic_;
    float emission_factor_;
};

}  // namespace scene::material::substitute

#endif
