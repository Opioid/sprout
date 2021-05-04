#ifndef SU_SCENE_MATERIAL_LIGHT_EMISSIONMAP_HPP
#define SU_SCENE_MATERIAL_LIGHT_EMISSIONMAP_HPP

#include "base/math/distribution/distribution_2d.hpp"
#include "scene/material/material.hpp"

namespace scene::material::light {

class Emissionmap : public Material {
  public:
    Emissionmap(Sampler_settings sampler_settings, bool two_sided);

    ~Emissionmap() override;

    material::Sample const& sample(float3_p wo, Ray const& ray, Renderstate const& rs,
                                   Filter filter, Sampler& sampler, Worker& worker) const override;

    float3 evaluate_radiance(float3_p wi, float3_p uvw, float extent, Filter filter,
                             Worker& worker) const override;

    float3 average_radiance(float area) const final;

    Radiance_sample radiance_sample(float3_p r3) const final;

    float emission_pdf(float3_p uvw, Filter filter, Worker const& worker) const final;

    void prepare_sampling(Shape const& shape, uint32_t part, Transformation const& trafo,
                          float area, bool importance_sampling, Threads& threads,
                          Scene const& scene) override;

    void set_emission_map(Texture const& emission_map);

    void set_emission_factor(float emission_factor);

  protected:
    Texture emission_map_;

    Distribution_2D distribution_;

    float emission_factor_;

    float total_weight_;

    float3 average_emission_;
};

}  // namespace scene::material::light

#endif
