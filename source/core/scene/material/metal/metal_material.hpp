#ifndef SU_CORE_SCENE_MATERIAL_METAL_MATERIAL_HPP
#define SU_CORE_SCENE_MATERIAL_METAL_MATERIAL_HPP

#include "scene/material/material.hpp"

namespace scene::material::metal {

class Material_isotropic : public Material {
  public:
    Material_isotropic(Sampler_settings sampler_settings, bool two_sided);

    void commit(Threads& threads, Scene const& scene) final;

    material::Sample const& sample(float3_p wo, Renderstate const& rs, Sampler& sampler,
                                   Worker& worker) const final;

    void set_normal_map(Texture const& normal_map);

    void set_ior(float3_p ior);
    void set_absorption(float3_p absorption);
    void set_roughness(float roughness);

    static size_t sample_size();

  protected:
    Texture normal_map_;

    float3 ior3_;
    float3 absorption_;

    float alpha_;
};

class Material_anisotropic : public Material {
  public:
    Material_anisotropic(Sampler_settings sampler_settings, bool two_sided);

    void commit(Threads& threads, Scene const& scene) final;

    material::Sample const& sample(float3_p wo, Renderstate const& rs, Sampler& sampler,
                                   Worker& worker) const final;

    void set_normal_map(Texture const& normal_map);
    void set_direction_map(Texture const& direction_map);

    void set_ior(float3_p ior);
    void set_absorption(float3_p absorption);
    void set_roughness(float2 roughness);

    static size_t sample_size();

  protected:
    Texture normal_map_;
    Texture direction_map_;

    float3 ior3_;
    float3 absorption_;

    float2 alpha_;
};

}  // namespace scene::material::metal

#endif
