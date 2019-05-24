#ifndef SU_CORE_IMAGE_TEXTURE_FLOAT1_HPP
#define SU_CORE_IMAGE_TEXTURE_FLOAT1_HPP

#include "image/typed_image_fwd.hpp"
#include "texture.hpp"

namespace image::texture {

template <typename T>
class alignas(64) Float1_t final : public Texture {
  public:
    Float1_t(Image const& image) noexcept;

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

    size_t image_num_bytes() const noexcept override final;

  private:
    T const& image_;
};

extern template class Float1_t<image::Float1>;
extern template class Float1_t<image::Float1_sparse>;

using Float1 = Float1_t<image::Float1>;

using Float1_sparse = Float1_t<image::Float1_sparse>;

}  // namespace image::texture

#endif
