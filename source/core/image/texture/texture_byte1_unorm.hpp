#ifndef SU_CORE_IMAGE_TEXTURE_BYTE1_UNORM_HPP
#define SU_CORE_IMAGE_TEXTURE_BYTE1_UNORM_HPP

#include "image/typed_image_fwd.hpp"

namespace image::texture {

class Byte1_unorm {
  public:
    Byte1_unorm(Byte1 const& image);

    Byte1 const& image() const;

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

    void gather_1(int3 const& xyz, int3 const& xyz1, float c[8]) const;
    void gather_2(int3 const& xyz, int3 const& xyz1, float2 c[8]) const;

  private:
    Byte1 const* image_;
};

}  // namespace image::texture

#endif
