#ifndef SU_CORE_SCENE_MATERIAL_METAL_MATERIAL_HPP
#define SU_CORE_SCENE_MATERIAL_METAL_MATERIAL_HPP

#include "scene/material/material.hpp"

namespace scene::material::metal {

class alignas(64) Material_isotropic : public Material {
  public:
    Material_isotropic(Sampler_settings const& sampler_settings, bool two_sided);

    void commit(Threads& threads, Scene const& scene) final;

    material::Sample const& sample(float3 const& wo, Ray const& ray, Renderstate const& rs,
                                   Filter filter, Sampler& sampler, Worker& worker) const final;

    void set_normal_map(Texture_adapter const& normal_map);

    void set_ior(float3 const& ior);
    void set_absorption(float3 const& absorption);
    void set_roughness(float roughness);

    static size_t sample_size();

  protected:
    Texture_adapter normal_map_;

    float3 ior3_;
    float3 absorption_;

    float alpha_;
};

class alignas(64) Material_anisotropic : public Material {
  public:
    Material_anisotropic(Sampler_settings const& sampler_settings, bool two_sided);

    void commit(Threads& threads, Scene const& scene) final;

    material::Sample const& sample(float3 const& wo, Ray const& ray, Renderstate const& rs,
                                   Filter filter, Sampler& sampler, Worker& worker) const final;

    void set_normal_map(Texture_adapter const& normal_map);
    void set_direction_map(Texture_adapter const& direction_map);

    void set_ior(float3 const& ior);
    void set_absorption(float3 const& absorption);
    void set_roughness(float2 roughness);

    static size_t sample_size();

  protected:
    Texture_adapter normal_map_;
    Texture_adapter direction_map_;

    float3 ior3_;
    float3 absorption_;

    float2 alpha_;
};

}  // namespace scene::material::metal

#endif
