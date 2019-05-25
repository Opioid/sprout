#ifndef SU_CORE_IMAGE_TEXTURE_BYTE2_SNORM_HPP
#define SU_CORE_IMAGE_TEXTURE_BYTE2_SNORM_HPP

#include "typed_texture.hpp"
#include "image/typed_image_fwd.hpp"

namespace image::texture {

class Byte2_snorm : public Typed_texture<Byte2> {
  public:
    Byte2_snorm(Image const& image) noexcept;

    float  at_1(int32_t i) const noexcept;
    float3 at_3(int32_t i) const noexcept;

    float  at_1(int32_t x, int32_t y) const noexcept;
    float2 at_2(int32_t x, int32_t y) const noexcept;
    float3 at_3(int32_t x, int32_t y) const noexcept;

    void gather_1(int4 const& xy_xy1, float c[4]) const noexcept;
    void gather_2(int4 const& xy_xy1, float2 c[4]) const noexcept;
    void gather_3(int4 const& xy_xy1, float3 c[4]) const noexcept;

    float  at_element_1(int32_t x, int32_t y, int32_t element) const noexcept;
    float2 at_element_2(int32_t x, int32_t y, int32_t element) const noexcept;
    float3 at_element_3(int32_t x, int32_t y, int32_t element) const noexcept;

    float  at_1(int32_t x, int32_t y, int32_t z) const noexcept;
    float2 at_2(int32_t x, int32_t y, int32_t z) const noexcept;
    float3 at_3(int32_t x, int32_t y, int32_t z) const noexcept;
    float4 at_4(int32_t x, int32_t y, int32_t z) const noexcept;
};



}  // namespace image::texture

#endif
