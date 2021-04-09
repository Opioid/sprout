#ifndef SU_SCENE_MATERIAL_LIGHT_EMISSIONMAP_ANIMATED_HPP
#define SU_SCENE_MATERIAL_LIGHT_EMISSIONMAP_ANIMATED_HPP

#include "base/math/distribution/distribution_2d.hpp"
#include "image/texture/texture.hpp"
#include "light_emissionmap.hpp"

namespace scene::material::light {

class Emissionmap_animated : public Emissionmap {
  public:
    Emissionmap_animated(Sampler_settings sampler_settings, bool two_sided);

    ~Emissionmap_animated() override;

    void simulate(uint64_t start, uint64_t end, uint64_t frame_length, Threads& threads,
                  Scene const& scene) final;

    material::Sample const& sample(float3_p wo, Ray const& ray, Renderstate const& rs,
                                   Filter filter, Sampler& sampler, Worker& worker) const override;

    float3 evaluate_radiance(float3_p wi, float3_p uvw, float extent, Filter filter,
                             Worker& worker) const final;

    void set_emission_map(Texture_adapter const& emission_map, uint64_t animation_duration);

  protected:
    uint64_t animation_duration_;
};

}  // namespace scene::material::light

#endif
