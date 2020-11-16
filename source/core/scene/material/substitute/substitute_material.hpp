#ifndef SU_CORE_SCENE_MATERIAL_SUBSTITUTE_MATERIAL_HPP
#define SU_CORE_SCENE_MATERIAL_SUBSTITUTE_MATERIAL_HPP

#include "substitute_base_material.hpp"

namespace scene::material::substitute {

class Material : public Material_base {
  public:
    Material(Sampler_settings sampler_settings, bool two_sided);

    material::Sample const& sample(float3_p wo, Ray const& ray, Renderstate const& rs,
                                   Filter filter, Sampler& sampler, Worker& worker) const final;

    static size_t sample_size();
};

class Checkers : public Material_base {
  public:
    Checkers(Sampler_settings sampler_settings, bool two_sided);

    material::Sample const& sample(float3_p wo, Ray const& ray, Renderstate const& rs,
                                   Filter filter, Sampler& sampler, Worker& worker) const final;

    void set_checkers(float3_p a, float3_p b, float scale);

    static size_t sample_size();

  private:
    float3 checkers_[2];

    float scale_;
};

}  // namespace scene::material::substitute

#endif
