#ifndef SU_CORE_SCENE_MATERIAL_VOLUMETRIC_MATERIAL_HPP
#define SU_CORE_SCENE_MATERIAL_VOLUMETRIC_MATERIAL_HPP

#include "scene/material/material.hpp"

namespace scene::material::volumetric {

class Material : public material::Material {
  public:
    Material(Sampler_settings sampler_settings);

    ~Material() override;

    material::Sample const& sample(float3_p wo, Renderstate const& rs, Sampler& sampler,
                                   Worker& worker) const final;

    float3 prepare_sampling(Shape const& shape, uint32_t part, Transformation const& trafo,
                            float area, bool importance_sampling, Threads& threads,
                            Scene const& scene) override;

    static size_t sample_size();
};

}  // namespace scene::material::volumetric

#endif
