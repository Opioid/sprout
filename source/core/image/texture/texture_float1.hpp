#ifndef SU_CORE_IMAGE_TEXTURE_FLOAT1_HPP
#define SU_CORE_IMAGE_TEXTURE_FLOAT1_HPP

#include "image/typed_image_fwd.hpp"

namespace image::texture {

template <typename T>
class Float1_t {
  public:
    Float1_t(T const& image);

    T const& image() const;

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

    void gather_1(int3 const& xyz, int3_p xyz1, float c[8]) const;
    void gather_2(int3 const& xyz, int3_p xyz1, float2 c[8]) const;

  private:
    T const& image_;
};

extern template class Float1_t<image::Float1>;
extern template class Float1_t<image::Float1_sparse>;

using Float1 = Float1_t<image::Float1>;

using Float1_sparse = Float1_t<image::Float1_sparse>;

}  // namespace image::texture

#endif
