#ifndef SU_CORE_SCENE_MATERIAL_METAL_MATERIAL_HPP
#define SU_CORE_SCENE_MATERIAL_METAL_MATERIAL_HPP

#include "scene/material/material.hpp"

namespace scene::material::metal {

class Material : public material::Material {
  public:
    Material(Sampler_settings sampler_settings, bool two_sided);

    void commit(Threads& threads, Scene const& scene) final;

    material::Sample const& sample(float3_p wo, Renderstate const& rs, Sampler& sampler,
                                   Worker& worker) const final;

    void set_normal_map(Texture const& normal_map);
    void set_rotation_map(Texture const& rotation_map);

    void set_ior(float3_p ior);
    void set_absorption(float3_p absorption);
    void set_roughness(float roughness, float anisotropy);
    void set_anisotropy_rotation(float angle);

    static size_t sample_size();

  protected:
    Texture normal_map_;
    Texture rotation_map_;

    float3 ior3_;
    float3 absorption_;

    float2 alpha_;

    float anisotropy_rotation_;
};

}  // namespace scene::material::metal

#endif
