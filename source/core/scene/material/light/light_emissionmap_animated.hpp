#ifndef SU_SCENE_MATERIAL_LIGHT_EMISSIONMAP_ANIMATED_HPP
#define SU_SCENE_MATERIAL_LIGHT_EMISSIONMAP_ANIMATED_HPP

#include "base/math/distribution/distribution_2d.hpp"
#include "image/texture/texture.hpp"
#include "light_emissionmap.hpp"

namespace scene::material::light {

class Emissionmap_animated : public Emissionmap {
  public:
    Emissionmap_animated(Sampler_settings const& sampler_settings, bool two_sided);

    ~Emissionmap_animated() override;

    void simulate(uint64_t start, uint64_t end, uint64_t frame_length, thread::Pool& threads,
                  Scene const& scene) final;

    material::Sample const& sample(float3 const& wo, Ray const& ray, Renderstate const& rs,
                                   Filter filter, Sampler& sampler,
                                   Worker const& worker) const override;

    float3 evaluate_radiance(float3 const& wi, float2 uv, float area, Filter filter,
                             Worker const& worker) const final;

    float opacity(float2 uv, uint64_t time, Filter filter, Worker const& worker) const final;

    void prepare_sampling(Shape const& shape, uint32_t part, uint64_t time,
                          Transformation const& transformation, float area,
                          bool importance_sampling, thread::Pool& threads,
                          Scene const& scene) final;

    bool is_animated() const final;

    void set_emission_map(Texture_adapter const& emission_map, uint64_t animation_duration);

    size_t num_bytes() const override;

  protected:
    uint64_t animation_duration_;

    int32_t element_;
};

}  // namespace scene::material::light

#endif
