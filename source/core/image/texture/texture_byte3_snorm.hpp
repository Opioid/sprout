#ifndef SU_CORE_IMAGE_TEXTURE_BYTE3_SNORM_HPP
#define SU_CORE_IMAGE_TEXTURE_BYTE3_SNORM_HPP

#include "image/typed_image_fwd.hpp"

namespace image::texture {

class Byte3_snorm {
  public:
    Byte3_snorm(Byte3 const& image);

    Description const& description() const;

    int3 dimensions() const {
        return dimensions_;
    }

    float  at_1(int32_t x, int32_t y) const;
    float2 at_2(int32_t x, int32_t y) const;
    float3 at_3(int32_t x, int32_t y) const;
    float4 at_4(int32_t x, int32_t y) const;

    void gather_1(int4_p xy_xy1, float c[4]) const;
    void gather_2(int4_p xy_xy1, float2 c[4]) const;
    void gather_3(int4_p xy_xy1, float3 c[4]) const;

    float  at_element_1(int32_t x, int32_t y, int32_t element) const;
    float2 at_element_2(int32_t x, int32_t y, int32_t element) const;
    float3 at_element_3(int32_t x, int32_t y, int32_t element) const;

    float  at_1(int32_t x, int32_t y, int32_t z) const;
    float2 at_2(int32_t x, int32_t y, int32_t z) const;
    float3 at_3(int32_t x, int32_t y, int32_t z) const;
    float4 at_4(int32_t x, int32_t y, int32_t z) const;

    void gather_1(int3_p xyz, int3_p xyz1, float c[8]) const;
    void gather_2(int3_p xyz, int3_p xyz1, float2 c[8]) const;

  private:
    Description const& description_;

    byte3 const* data_;

    int3 const dimensions_;
};

}  // namespace image::texture

#endif
