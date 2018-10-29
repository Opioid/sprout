#include "texture_float_2.hpp"
#include "base/math/vector4.inl"
#include "image/typed_image.inl"

namespace image::texture {

Float2::Float2(std::shared_ptr<Image> const& image) noexcept
    : Texture(image), image_(*static_cast<const image::Float2*>(image.get())) {}

float Float2::at_1(int32_t i) const noexcept {
    return image_.load(i)[0];
}

float3 Float2::at_3(int32_t i) const noexcept {
    return float3(image_.load(i), 0.f);
}

float Float2::at_1(int32_t x, int32_t y) const noexcept {
    return image_.load(x, y)[0];
}

float2 Float2::at_2(int32_t x, int32_t y) const noexcept {
    return image_.load(x, y);
}

float3 Float2::at_3(int32_t x, int32_t y) const noexcept {
    return float3(image_.load(x, y), 0.f);
}

void Float2::gather_1(int4 const& xy_xy1, float c[4]) const noexcept {
    float2 v[4];
    image_.gather(xy_xy1, v);

    c[0] = v[0][0];
    c[1] = v[1][0];
    c[2] = v[2][0];
    c[3] = v[3][0];
}

void Float2::gather_2(int4 const& xy_xy1, float2 c[4]) const noexcept {
    image_.gather(xy_xy1, c);
}

void Float2::gather_3(int4 const& xy_xy1, float3 c[4]) const noexcept {
    float2 v[4];
    image_.gather(xy_xy1, v);

    c[0] = float3(v[0], 0.f);
    c[1] = float3(v[1], 0.f);
    c[2] = float3(v[2], 0.f);
    c[3] = float3(v[3], 0.f);
}

float Float2::at_element_1(int32_t x, int32_t y, int32_t element) const noexcept {
    return image_.at_element(x, y, element)[0];
}

float2 Float2::at_element_2(int32_t x, int32_t y, int32_t element) const noexcept {
    return image_.at_element(x, y, element);
}

float3 Float2::at_element_3(int32_t x, int32_t y, int32_t element) const noexcept {
    return float3(image_.at_element(x, y, element), 0.f);
}

float Float2::at_1(int32_t x, int32_t y, int32_t z) const noexcept {
    return image_.load(x, y, z)[0];
}

float2 Float2::at_2(int32_t x, int32_t y, int32_t z) const noexcept {
    return image_.load(x, y, z);
}

float3 Float2::at_3(int32_t x, int32_t y, int32_t z) const noexcept {
    return float3(image_.load(x, y, z), 0.f);
}

}  // namespace image::texture
