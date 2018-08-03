#pragma once

#include "sampler_3d.hpp"

namespace image::texture::sampler {

template <typename Address_mode>
class Nearest_3D : public Sampler_3D {
  public:
    virtual float sample_1(Texture const& texture, float3 const& uvw) const noexcept override final;
    virtual float2 sample_2(Texture const& texture, float3 const& uvw) const
        noexcept override final;
    virtual float3 sample_3(Texture const& texture, float3 const& uvw) const
        noexcept override final;

    virtual float3 address(float3 const& uvw) const noexcept override final;

  private:
    static int3 map(Texture const& texture, float3 const& uvw) noexcept;
};

}  // namespace image::texture::sampler
