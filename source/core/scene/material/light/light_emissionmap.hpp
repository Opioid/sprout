#ifndef SU_SCENE_MATERIAL_LIGHT_EMISSIONMAP_HPP
#define SU_SCENE_MATERIAL_LIGHT_EMISSIONMAP_HPP

#include "base/math/distribution_2d.hpp"
#include "scene/material/material.hpp"

namespace scene::material::light {

class Emissionmap final : public Material {
  public:
    Emissionmap(Sampler_settings sampler_settings, bool two_sided);

    ~Emissionmap() override;

    material::Sample const& sample(float3_p wo, Renderstate const& rs, Sampler& sampler,
                                   Worker& worker) const final;

    float3 evaluate_radiance(float3_p wi, float3_p n, float3_p uvw, float extent, Filter filter,
                             Worker const& worker) const final;

    Radiance_sample radiance_sample(float3_p r3) const final;

    float emission_pdf(float3_p uvw, Worker const& worker) const final;

    float3 prepare_sampling(Shape const& shape, uint32_t part, Transformation const& trafo,
                            float area, Scene const& scene, Threads& threads) final;

    void set_emission_map(Texture const& emission_map);

    void set_emission_factor(float emission_factor);

    image::Description useful_texture_description(Scene const& scene) const final;

  protected:
    Texture emission_map_;

    Distribution_2D distribution_;

    float emission_factor_;

    float total_weight_;

    float3 average_emission_;
};

}  // namespace scene::material::light

#endif
