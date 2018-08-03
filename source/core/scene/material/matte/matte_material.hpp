#pragma once

#include "scene/material/material.hpp"

namespace scene::material::matte {

class Material : public material::Material {
  public:
    Material(Sampler_settings const& sampler_settings, bool two_sided);

    virtual material::Sample const& sample(float3 const& wo, Renderstate const& rs,
                                           Sampler_filter filter, sampler::Sampler& sampler,
                                           Worker const& worker,
                                           uint32_t      depth) const override final;

    virtual float ior() const override final;

    virtual size_t num_bytes() const override final;

    void set_color(float3 const& color);

    static size_t sample_size();

  private:
    float3 color_;
};

}  // namespace scene::material::matte
