#pragma once

#include "base/math/distribution/distribution_2d.hpp"
#include "image/texture/texture.hpp"
#include "scene/material/light/light_emissionmap_animated.hpp"

namespace scene::material::display {

class Emissionmap_animated : public light::Emissionmap_animated {
  public:
    Emissionmap_animated(Sampler_settings const& sampler_settings, bool two_sided);

    material::Sample const& sample(float3 const& wo, Ray const& ray, Renderstate const& rs,
                                   Filter filter, Sampler& sampler, Worker& worker) const final;

    float ior() const final;

    void set_roughness(float roughness);
    void set_ior(float ior);

    size_t num_bytes() const final;

  private:
    float alpha_;

    float ior_;

    float f0_;
};

}  // namespace scene::material::display
