#ifndef SU_CORE_SCENE_MATERIAL_SUBSTITUTE_MATERIAL_HPP
#define SU_CORE_SCENE_MATERIAL_SUBSTITUTE_MATERIAL_HPP

#include "substitute_base_material.hpp"

namespace scene::material::substitute {

class alignas(16) Material : public Material_base {
  public:
    Material(Sampler_settings const& sampler_settings, bool two_sided);

    material::Sample const& sample(float3 const& wo, Ray const& ray, Renderstate const& rs,
                                   Filter filter, Sampler& sampler, Worker& worker) const final;

    static size_t sample_size();
};

class alignas(16) Frozen : public Material_base {
  public:
    Frozen(Sampler_settings const& sampler_settings, bool two_sided);

    material::Sample const& sample(float3 const& wo, Ray const& ray, Renderstate const& rs,
                                   Filter filter, Sampler& sampler, Worker& worker) const final;

    void set_snow_normal_map(Texture_adapter const& normal_map);

    void set_snow_mask(Texture_adapter const& mask);

    static size_t sample_size();

  private:
    Texture_adapter snow_normal_map_;

    Texture_adapter snow_mask_;
};

}  // namespace scene::material::substitute

#endif
