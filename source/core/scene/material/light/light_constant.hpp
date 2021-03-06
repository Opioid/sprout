#ifndef SU_SCENE_MATERIAL_LIGHT_CONSTANT_HPP
#define SU_SCENE_MATERIAL_LIGHT_CONSTANT_HPP

#include "scene/light/emittance.hpp"
#include "scene/material/material.hpp"

namespace scene::material::light {

class Constant : public Material {
  public:
    Constant(Sampler_settings sampler_settings, bool two_sided);

    material::Sample const& sample(float3_p wo, Renderstate const& rs, Sampler& sampler,
                                   Worker& worker) const final;

    float3 evaluate_radiance(float3_p wi, float3_p n, float3_p uvw, float extent, Filter filter,
                             Worker const& worker) const final;

    float3 prepare_sampling(Shape const& shape, uint32_t part, Transformation const& trafo,
                            float area, Scene const& scene, Threads& threads) final;

    ::light::Emittance& emittance();

    static size_t sample_size();

  private:
    ::light::Emittance emittance_;
};

}  // namespace scene::material::light

#endif
