#ifndef SU_SCENE_MATERIAL_DISPLAY_EMISSIONMAP_HPP
#define SU_SCENE_MATERIAL_DISPLAY_EMISSIONMAP_HPP

#include "scene/material/light/light_emissionmap.hpp"

namespace scene::material::display {

class Emissionmap : public light::Emissionmap {
  public:
    Emissionmap(Sampler_settings const& sampler_settings, bool two_sided);

    material::Sample const& sample(float3 const& wo, Ray const& ray, Renderstate const& rs,
                                   Filter filter, Sampler& sampler, Worker& worker) const final;

    float ior() const final;

    size_t num_bytes() const final;

    void set_roughness(float roughness);
    void set_ior(float ior);

  private:
    float alpha_;

    float ior_;

    float f0_;
};

}  // namespace scene::material::display

#endif
