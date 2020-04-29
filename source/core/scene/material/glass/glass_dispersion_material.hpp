#ifndef SU_CORE_SCENE_MATERIAL_GLASS_DISPERSION_MATERIAL_HPP
#define SU_CORE_SCENE_MATERIAL_GLASS_DISPERSION_MATERIAL_HPP

#include "glass_material.hpp"

namespace scene::material::glass {

class Glass_dispersion final : public Glass {
  public:
    Glass_dispersion(Sampler_settings const& sampler_settings);

    material::Sample const& sample(float3 const& wo, Ray const& ray, Renderstate const& rs,
                                   Filter filter, Sampler& sampler, Worker& worker) const final;

    void set_abbe(float abbe);

    static size_t sample_size();

  private:
    float abbe_;
};

}  // namespace scene::material::glass

#endif
