#ifndef SU_CORE_IMAGE_TEXTURE_TEXTURE_HPP
#define SU_CORE_IMAGE_TEXTURE_TEXTURE_HPP

#include <memory>
#include "base/math/vector3.hpp"

namespace image {

class Image;

namespace texture {

class Texture {
  public:
    Texture(std::shared_ptr<Image> const& image) noexcept;

    virtual ~Texture() noexcept;

    Image const& image() const noexcept;

    int32_t num_channels() const noexcept;
    int32_t num_elements() const noexcept;

    int2        dimensions_2() const noexcept;
    int3 const& dimensions_3() const noexcept;

    int2        back_2() const noexcept;
    int3 const& back_3() const noexcept;

    float2        dimensions_float2() const noexcept;
    float3 const& dimensions_float3() const noexcept;

    virtual float  at_1(int32_t i) const noexcept = 0;
    virtual float3 at_3(int32_t i) const noexcept = 0;

    virtual float  at_1(int32_t x, int32_t y) const noexcept = 0;
    virtual float2 at_2(int32_t x, int32_t y) const noexcept = 0;
    virtual float3 at_3(int32_t x, int32_t y) const noexcept = 0;

    virtual void gather_1(int4 const& xy_xy1, float c[4]) const noexcept  = 0;
    virtual void gather_2(int4 const& xy_xy1, float2 c[4]) const noexcept = 0;
    virtual void gather_3(int4 const& xy_xy1, float3 c[4]) const noexcept = 0;

    virtual float  at_element_1(int32_t x, int32_t y, int32_t element) const noexcept = 0;
    virtual float2 at_element_2(int32_t x, int32_t y, int32_t element) const noexcept = 0;
    virtual float3 at_element_3(int32_t x, int32_t y, int32_t element) const noexcept = 0;

    virtual float  at_1(int32_t x, int32_t y, int32_t z) const noexcept = 0;
    virtual float2 at_2(int32_t x, int32_t y, int32_t z) const noexcept = 0;
    virtual float3 at_3(int32_t x, int32_t y, int32_t z) const noexcept = 0;

    float3 average_3() const noexcept;
    float3 average_3(int32_t element) const noexcept;

  protected:
    std::shared_ptr<Image> untyped_image_;

    int3 back_;

    float3 dimensions_float_;
};

}  // namespace texture
}  // namespace image

#endif
