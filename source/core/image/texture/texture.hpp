#ifndef SU_CORE_IMAGE_TEXTURE_TEXTURE_HPP
#define SU_CORE_IMAGE_TEXTURE_TEXTURE_HPP

#include <memory>
#include "base/math/vector3.hpp"

namespace image {

class Image;

namespace texture {

class Texture {
  public:
    Texture(std::shared_ptr<Image> const& image);
    virtual ~Texture();

    const Image* image() const;

    int32_t       num_channels() const;
    int32_t       width() const;
    int2          dimensions_2() const;
    int3          dimensions_3() const;
    int2          back_2() const;
    int3 const&   back_3() const;
    float2        dimensions_float2() const;
    float3 const& dimensions_float3() const;
    int32_t       num_elements() const;

    virtual float  at_1(int32_t i) const = 0;
    virtual float3 at_3(int32_t i) const = 0;

    virtual float  at_1(int32_t x, int32_t y) const = 0;
    virtual float2 at_2(int32_t x, int32_t y) const = 0;
    virtual float3 at_3(int32_t x, int32_t y) const = 0;
    virtual float4 at_4(int32_t x, int32_t y) const = 0;

    virtual float4 gather_1(int4 const& xy_xy1) const              = 0;
    virtual void   gather_2(int4 const& xy_xy1, float2 c[4]) const = 0;
    virtual void   gather_3(int4 const& xy_xy1, float3 c[4]) const = 0;

    virtual float  at_element_1(int32_t x, int32_t y, int32_t element) const = 0;
    virtual float2 at_element_2(int32_t x, int32_t y, int32_t element) const = 0;
    virtual float3 at_element_3(int32_t x, int32_t y, int32_t element) const = 0;
    virtual float4 at_element_4(int32_t x, int32_t y, int32_t element) const = 0;

    virtual float  at_1(int32_t x, int32_t y, int32_t z) const = 0;
    virtual float2 at_2(int32_t x, int32_t y, int32_t z) const = 0;
    virtual float3 at_3(int32_t x, int32_t y, int32_t z) const = 0;
    virtual float4 at_4(int32_t x, int32_t y, int32_t z) const = 0;

    float3 average_3() const;
    float3 average_3(int32_t element) const;

    float4 average_4() const;

  protected:
    std::shared_ptr<Image> untyped_image_;

    int3   back_;
    float3 dimensions_float_;
};

}  // namespace texture
}  // namespace image

#endif
