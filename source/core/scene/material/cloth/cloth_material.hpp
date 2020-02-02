#pragma once

#include "scene/material/material.hpp"

namespace scene::material::cloth {

class Material : public material::Material {
  public:
    Material(Sampler_settings const& sampler_settings, bool two_sided);

    material::Sample const& sample(float3 const& wo, Ray const& ray, Renderstate const& rs,
                                   Filter filter, Sampler& sampler,
                                   Worker const& worker) const final;

    float ior() const final;

    size_t num_bytes() const final;

    void set_color_map(Texture_adapter const& color_map);
    void set_normal_map(Texture_adapter const& normal_map);

    void set_color(float3 const& color);

    static size_t sample_size();

  private:
    Texture_adapter color_map_;
    Texture_adapter normal_map_;

    float3 color_;
};

}  // namespace scene::material::cloth
