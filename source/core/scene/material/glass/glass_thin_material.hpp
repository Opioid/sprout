#ifndef SU_CORE_SCENE_MATERIAL_GLASS_THIN_MATERIAL_HPP
#define SU_CORE_SCENE_MATERIAL_GLASS_THIN_MATERIAL_HPP

#include "scene/material/material.hpp"

namespace scene::material::glass {

class Glass_thin : public Material {
  public:
    Glass_thin(Sampler_settings sampler_settings);

    material::Sample const& sample(float3_p wo, Renderstate const& rs, Sampler& sampler,
                                   Worker& worker) const final;

    bool visibility(float3_p wi, float3_p n, float2 uv, Filter filter, Worker const& worker,
                    float3& v) const final;

    void set_normal_map(Texture const& normal_map);

    void set_refraction_color(float3_p color);

    void set_thickness(float thickness);

    static size_t sample_size();

  private:
    Texture normal_map_;

    float3 refraction_color_;

    float thickness_;
};

}  // namespace scene::material::glass

#endif
