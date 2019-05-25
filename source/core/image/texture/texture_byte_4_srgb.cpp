#include "texture_byte_4_srgb.hpp"
#include "base/math/vector4.inl"
#include "image/typed_image.hpp"
#include "texture_encoding.hpp"

namespace image::texture {

Byte4_sRGB::Byte4_sRGB(Image const& image) noexcept
    : Typed_texture<Byte4>(static_cast<Byte4 const&>(image)) {}

float Byte4_sRGB::at_1(int32_t i) const noexcept {
    auto const value = image_.load(i);
    return encoding::cached_srgb_to_float(value[0]);
}

float3 Byte4_sRGB::at_3(int32_t i) const noexcept {
    auto const value = image_.load(i);
    return float3(encoding::cached_srgb_to_float(value[0]),
                  encoding::cached_srgb_to_float(value[1]),
                  encoding::cached_srgb_to_float(value[2]));
}

float Byte4_sRGB::at_1(int32_t x, int32_t y) const noexcept {
    auto const value = image_.load(x, y);
    return encoding::cached_srgb_to_float(value[0]);
}

float2 Byte4_sRGB::at_2(int32_t x, int32_t y) const noexcept {
    auto const value = image_.load(x, y);
    return encoding::cached_srgb_to_float2(value);
}

float3 Byte4_sRGB::at_3(int32_t x, int32_t y) const noexcept {
    auto const value = image_.load(x, y);
    return encoding::cached_srgb_to_float3(value);
}

void Byte4_sRGB::gather_1(int4 const& xy_xy1, float c[4]) const noexcept {
    byte4 v[4];
    image_.gather(xy_xy1, v);

    encoding::cached_srgb_to_float(v, c);
}

void Byte4_sRGB::gather_2(int4 const& xy_xy1, float2 c[4]) const noexcept {
    byte4 v[4];
    image_.gather(xy_xy1, v);

    encoding::cached_srgb_to_float(v, c);
}

void Byte4_sRGB::gather_3(int4 const& xy_xy1, float3 c[4]) const noexcept {
    byte4 v[4];
    image_.gather(xy_xy1, v);

    encoding::cached_srgb_to_float(v, c);
}

float Byte4_sRGB::at_element_1(int32_t x, int32_t y, int32_t element) const noexcept {
    auto const value = image_.load_element(x, y, element);
    return encoding::cached_srgb_to_float(value[0]);
}

float2 Byte4_sRGB::at_element_2(int32_t x, int32_t y, int32_t element) const noexcept {
    auto const value = image_.load_element(x, y, element);
    return float2(encoding::cached_srgb_to_float(value[0]),
                  encoding::cached_srgb_to_float(value[1]));
}

float3 Byte4_sRGB::at_element_3(int32_t x, int32_t y, int32_t element) const noexcept {
    auto const value = image_.load_element(x, y, element);
    return encoding::cached_srgb_to_float3(value.xyz());
}

float Byte4_sRGB::at_1(int32_t x, int32_t y, int32_t z) const noexcept {
    auto const value = image_.load(x, y, z);
    return encoding::cached_srgb_to_float(value[0]);
}

float2 Byte4_sRGB::at_2(int32_t x, int32_t y, int32_t z) const noexcept {
    auto const value = image_.load(x, y, z);
    return float2(encoding::cached_srgb_to_float(value[0]),
                  encoding::cached_srgb_to_float(value[1]));
}

float3 Byte4_sRGB::at_3(int32_t x, int32_t y, int32_t z) const noexcept {
    auto const value = image_.load(x, y, z);
    return encoding::cached_srgb_to_float3(value);
}

float4 Byte4_sRGB::at_4(int32_t x, int32_t y, int32_t z) const noexcept {
    auto const value = image_.load(x, y, z);
    return encoding::cached_srgb_to_float4(value);
}

}  // namespace image::texture
