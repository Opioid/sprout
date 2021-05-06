#ifndef SU_CORE_SCENE_MATERIAL_GLASS_DISPERSION_MATERIAL_HPP
#define SU_CORE_SCENE_MATERIAL_GLASS_DISPERSION_MATERIAL_HPP

#include "glass_material.hpp"

namespace scene::material::glass {

class Glass_dispersion final : public Glass {
  public:
    Glass_dispersion(Sampler_settings sampler_settings);

    material::Sample const& sample(float3_p wo, Renderstate const& rs, Sampler& sampler,
                                   Worker& worker) const final;

    void set_abbe(float abbe);

    static size_t sample_size();

  private:
    float abbe_;
};

}  // namespace scene::material::glass

#endif
