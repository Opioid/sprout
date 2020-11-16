#ifndef SU_SCENE_MATERIAL_DISPLAY_EMISSIONMAP_HPP
#define SU_SCENE_MATERIAL_DISPLAY_EMISSIONMAP_HPP

#include "scene/material/light/light_emissionmap.hpp"

namespace scene::material::display {

class Emissionmap : public light::Emissionmap {
  public:
    Emissionmap(Sampler_settings sampler_settings, bool two_sided);

    material::Sample const& sample(float3_p wo, Ray const& ray, Renderstate const& rs,
                                   Filter filter, Sampler& sampler, Worker& worker) const final;

    void set_roughness(float roughness);

  private:
    float alpha_;
};

}  // namespace scene::material::display

#endif
