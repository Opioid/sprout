#pragma once

#include "base/math/distribution/distribution_2d.hpp"
#include "image/texture/texture.hpp"
#include "scene/material/light/light_emissionmap_animated.hpp"

namespace scene::material::display {

class Emissionmap_animated : public light::Emissionmap_animated {
  public:
    Emissionmap_animated(Sampler_settings const& sampler_settings, bool two_sided,
                         Texture_adapter const& emission_map, float emission_factor,
                         uint64_t animation_duration) noexcept;

    material::Sample const& sample(float3 const& wo, Ray const& ray, Renderstate const& rs,
                                   Filter filter, sampler::Sampler& sampler, Worker const& worker,
                                   uint32_t sample_level) const noexcept override final;

    float ior() const noexcept override final;

    size_t num_bytes() const noexcept override final;

    void set_roughness(float roughness) noexcept;
    void set_ior(float ior) noexcept;

  private:
    float alpha_;

    float ior_;

    float f0_;
};

}  // namespace scene::material::display
