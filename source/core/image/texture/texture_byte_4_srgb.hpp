#ifndef SU_CORE_IMAGE_TEXTURE_BYTE3_SRGB_HPP
#define SU_CORE_IMAGE_TEXTURE_BYTE3_SRGB_HPP

#include "image/typed_image_fwd.hpp"
#include "texture.hpp"

namespace image::texture {

class Byte4_sRGB final : public Texture {
  public:
    Byte4_sRGB(Image const& image) noexcept;

    float  at_1(int32_t i) const noexcept override final;
    float3 at_3(int32_t i) const noexcept override final;

    float  at_1(int32_t x, int32_t y) const noexcept override final;
    float2 at_2(int32_t x, int32_t y) const noexcept override final;
    float3 at_3(int32_t x, int32_t y) const noexcept override final;

    void gather_1(int4 const& xy_xy1, float c[4]) const noexcept override final;
    void gather_2(int4 const& xy_xy1, float2 c[4]) const noexcept override final;
    void gather_3(int4 const& xy_xy1, float3 c[4]) const noexcept override final;

    float  at_element_1(int32_t x, int32_t y, int32_t element) const noexcept override final;
    float2 at_element_2(int32_t x, int32_t y, int32_t element) const noexcept override final;
    float3 at_element_3(int32_t x, int32_t y, int32_t element) const noexcept override final;

    float  at_1(int32_t x, int32_t y, int32_t z) const noexcept override final;
    float2 at_2(int32_t x, int32_t y, int32_t z) const noexcept override final;
    float3 at_3(int32_t x, int32_t y, int32_t z) const noexcept override final;
    float4 at_4(int32_t x, int32_t y, int32_t z) const noexcept override final;

  private:
    Byte4 const& image_;
};

}  // namespace image::texture

#endif
