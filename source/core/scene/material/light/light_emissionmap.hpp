#ifndef SU_SCENE_MATERIAL_LIGHT_EMISSIONMAP_HPP
#define SU_SCENE_MATERIAL_LIGHT_EMISSIONMAP_HPP

#include "base/math/distribution/distribution_2d.hpp"
#include "scene/material/material.hpp"

namespace scene::material::light {

class alignas(64) Emissionmap : public Material {
  public:
    Emissionmap(Sampler_settings const& sampler_settings, bool two_sided);

    ~Emissionmap() override;

    material::Sample const& sample(float3 const& wo, Ray const& ray, Renderstate const& rs,
                                   Filter filter, Sampler& sampler, Worker& worker) const override;

    float3 evaluate_radiance(float3 const& wi, float2 uv, float area, Filter filter,
                             Worker const& worker) const override;

    float3 average_radiance(float area, Scene const& scene) const final;

    bool has_emission_map() const final;

    Sample_2D radiance_sample(float2 r2) const final;

    float emission_pdf(float2 uv, Filter filter, Worker const& worker) const final;

    void prepare_sampling(Shape const& shape, uint32_t part, uint64_t time,
                          Transformation const& transformation, float area,
                          bool importance_sampling, thread::Pool& threads,
                          Scene const& scene) override;

    void set_emission_map(Texture_adapter const& emission_map);

    void set_emission_factor(float emission_factor);

    size_t num_bytes() const override;

  protected:
    void prepare_sampling_internal(Shape const& shape, int32_t element, bool importance_sampling,
                                   thread::Pool& threads, Scene const& scene);

    Texture_adapter emission_map_;

    Distribution_2D distribution_;

    float emission_factor_;

    float total_weight_;

    float3 average_emission_;
};

}  // namespace scene::material::light

#endif
