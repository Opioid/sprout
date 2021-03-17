#ifndef SU_CORE_SCENE_MATERIAL_COATING_MATERIAL_HPP
#define SU_CORE_SCENE_MATERIAL_COATING_MATERIAL_HPP

#include "substitute_base_material.hpp"

namespace scene::material::substitute {

template <typename Coating>
class Material_coating : public Material_base {
  public:
    Material_coating(Sampler_settings sampler_settings, bool two_sided);

    void set_coating_thickness_map(Texture_adapter const& thickness_map);

    void set_coating_normal_map(Texture_adapter const& normal_map);

    void set_coating_thickness(float thickness);

  protected:
    template <typename Sample>
    void set_coating_basis(float3_p wo, Renderstate const& rs, Texture_sampler_2D const& sampler,
                           Worker const& worker, Sample& sample) const;

    Texture_adapter coating_thickness_map_;
    Texture_adapter coating_normal_map_;

    Coating coating_;
};

struct Clearcoat_data {
    float3 absorption_coef;

    float thickness;
    float ior;
    float alpha;
};

class Material_clearcoat : public Material_coating<Clearcoat_data> {
  public:
    Material_clearcoat(Sampler_settings sampler_settings, bool two_sided);

    float3 evaluate_radiance(float3_p wi, float3_p uvw, float volume, Filter filter,
                             Worker const& worker) const final;

    material::Sample const& sample(float3_p wo, Ray const& ray, Renderstate const& rs,
                                   Filter filter, Sampler& sampler, Worker& worker) const override;

    void set_coating_attenuation(float3_p attenuation_color, float distance);

    void set_coating_ior(float ior);

    void set_coating_roughness(float roughness);

    static size_t sample_size();
};

struct Thinfilm_data {
    float ior;
    float alpha;
    float thickness;
};

class Material_thinfilm : public Material_coating<Thinfilm_data> {
  public:
    Material_thinfilm(Sampler_settings sampler_settings, bool two_sided);

    material::Sample const& sample(float3_p wo, Ray const& ray, Renderstate const& rs,
                                   Filter filter, Sampler& sampler, Worker& worker) const final;

    void set_thinfilm(float ior, float roughness, float thickness);

    static size_t sample_size();
};

}  // namespace scene::material::substitute

#endif
