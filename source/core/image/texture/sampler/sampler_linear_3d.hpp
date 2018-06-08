#ifndef SU_CORE_IMAGE_TEXTURE_SAMPLER_LINEAR_3D_HPP
#define SU_CORE_IMAGE_TEXTURE_SAMPLER_LINEAR_3D_HPP

#include "sampler_3d.hpp"

namespace image::texture::sampler {

template <typename Address_mode>
class Linear_3D : public Sampler_3D {
  public:
    virtual float  sample_1(Texture const& texture, f_float3 uvw) const override final;
    virtual float2 sample_2(Texture const& texture, f_float3 uvw) const override final;
    virtual float3 sample_3(Texture const& texture, f_float3 uvw) const override final;

    virtual float3 address(f_float3 uvw) const override final;

  private:
    static float3 map(Texture const& texture, f_float3 uvw, int3& xyz, int3& xyz1);
};

}  // namespace image::texture::sampler

#endif
