#ifndef SU_CORE_IMAGE_TEXTURE_SAMPLER_NEAREST_2D
#define SU_CORE_IMAGE_TEXTURE_SAMPLER_NEAREST_2D

#include "sampler_2d.hpp"

namespace image::texture::sampler {

template <typename Address_mode_U, typename Address_mode_V>
class Nearest_2D final : public Sampler_2D {
  public:
    float  sample_1(Texture const& texture, float2 uv) const noexcept override final;
    float2 sample_2(Texture const& texture, float2 uv) const noexcept override final;
    float3 sample_3(Texture const& texture, float2 uv) const noexcept override final;

    float sample_1(Texture const& texture, float2 uv, int32_t element) const
        noexcept override final;

    float2 sample_2(Texture const& texture, float2 uv, int32_t element) const
        noexcept override final;

    float3 sample_3(Texture const& texture, float2 uv, int32_t element) const
        noexcept override final;

    float2 address(float2 uv) const noexcept override final;

  private:
    static int2 map(Texture const& texture, float2 uv) noexcept;
};

}  // namespace image::texture::sampler

#endif
