#ifndef SU_CORE_IMAGE_TEXTURE_BYTE3_SRGB_HPP
#define SU_CORE_IMAGE_TEXTURE_BYTE3_SRGB_HPP

#include "image/typed_image_fwd.hpp"

namespace image::texture {

class Byte3_sRGB {
  public:
    Byte3_sRGB(Byte3 const& image);

    Byte3 const& image() const;

    float  at_1(int32_t x, int32_t y) const;
    float2 at_2(int32_t x, int32_t y) const;
    float3 at_3(int32_t x, int32_t y) const;
    float4 at_4(int32_t x, int32_t y) const;

    void gather_1(int4 const& xy_xy1, float c[4]) const;
    void gather_2(int4 const& xy_xy1, float2 c[4]) const;
    void gather_3(int4 const& xy_xy1, float3 c[4]) const;

    float  at_element_1(int32_t x, int32_t y, int32_t element) const;
    float2 at_element_2(int32_t x, int32_t y, int32_t element) const;
    float3 at_element_3(int32_t x, int32_t y, int32_t element) const;

    float  at_1(int32_t x, int32_t y, int32_t z) const;
    float2 at_2(int32_t x, int32_t y, int32_t z) const;
    float3 at_3(int32_t x, int32_t y, int32_t z) const;
    float4 at_4(int32_t x, int32_t y, int32_t z) const;

  private:
    Byte3 const& image_;
};

}  // namespace image::texture

#endif
