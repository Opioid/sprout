#ifndef SU_CORE_IMAGE_TEXTURE_SAMPLER_3D
#define SU_CORE_IMAGE_TEXTURE_SAMPLER_3D

#include "base/math/vector.hpp"

namespace image::texture {

class Texture;

namespace sampler {

class Sampler_3D {
  public:
    virtual ~Sampler_3D() {}

    virtual float  sample_1(Texture const& texture, f_float3 uvw) const noexcept = 0;
    virtual float2 sample_2(Texture const& texture, f_float3 uvw) const noexcept = 0;
    virtual float3 sample_3(Texture const& texture, f_float3 uvw) const noexcept = 0;

    virtual float3 address(f_float3 uvw) const noexcept = 0;
};

}  // namespace sampler
}  // namespace image::texture

#endif
