#ifndef SU_CORE_SCENE_MATERIAL_COATING_MATERIAL_HPP
#define SU_CORE_SCENE_MATERIAL_COATING_MATERIAL_HPP

#include "substitute_base_material.hpp"

namespace scene::material::substitute {

template <typename Coating>
class Material_coating : public Material_base {
  public:
    Material_coating(Sampler_settings const& sampler_settings, bool two_sided) noexcept;

    size_t num_bytes() const noexcept override;

    void set_coating_thickness_map(Texture_adapter const& thickness_map) noexcept;
    void set_coating_normal_map(Texture_adapter const& normal_map) noexcept;

    void set_coating_thickness(float thickness) noexcept;

  protected:
    template <typename Sample>
    void set_coating_basis(float3 const& wo, Renderstate const& rs,
                           Texture_sampler_2D const& sampler, Worker const& worker,
                           Sample& sample) const noexcept;

    Texture_adapter coating_thickness_map_;
    Texture_adapter coating_normal_map_;

    Coating coating_;
};

struct Clearcoat_data {
    float3 absorption_coefficient;

    float thickness;
    float ior;
    float alpha;
};

class Material_clearcoat : public Material_coating<Clearcoat_data> {
  public:
    Material_clearcoat(Sampler_settings const& sampler_settings, bool two_sided) noexcept;

    float3 evaluate_radiance(float3 const& wi, float2 uv, float area, Filter filter,
                             Worker const& worker) const noexcept override final;

    material::Sample const& sample(float3 const& wo, Ray const& ray, Renderstate const& rs,
                                   Filter filter, Sampler& sampler, Worker const& worker) const
        noexcept override;

    void set_coating_attenuation(float3 const& absorption_color, float distance) noexcept;

    void set_coating_ior(float ior) noexcept;

    void set_coating_roughness(float roughness) noexcept;

    static size_t sample_size() noexcept;
};

struct Thinfilm_data {
    float ior;
    float alpha;
    float thickness;
};

class Material_thinfilm : public Material_coating<Thinfilm_data> {
  public:
    Material_thinfilm(Sampler_settings const& sampler_settings, bool two_sided) noexcept;

    material::Sample const& sample(float3 const& wo, Ray const& ray, Renderstate const& rs,
                                   Filter filter, Sampler& sampler, Worker const& worker) const
        noexcept override final;

    void set_thinfilm(float ior, float roughness, float thickness) noexcept;

    static size_t sample_size() noexcept;
};

}  // namespace scene::material::substitute

#endif
