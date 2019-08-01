#include "texture_byte_2_unorm.hpp"
#include "base/math/vector4.inl"
#include "image/typed_image.hpp"
#include "texture_encoding.hpp"

namespace image::texture {

Byte2_unorm::Byte2_unorm(Byte2 const& image) noexcept : image_(image) {}

Byte2 const& Byte2_unorm::image() const noexcept {
    return image_;
}

int32_t Byte2_unorm::num_elements() const noexcept {
    return image_.description().num_elements;
}

int2 Byte2_unorm::dimensions_2() const noexcept {
    return image_.description().dimensions.xy();
}

int3 const& Byte2_unorm::dimensions_3() const noexcept {
    return image_.description().dimensions;
}

float Byte2_unorm::at_1(int32_t i) const noexcept {
    auto value = image_.load(i);
    return encoding::cached_unorm_to_float(value[0]);
}

float3 Byte2_unorm::at_3(int32_t i) const noexcept {
    auto value = image_.load(i);
    return float3(encoding::cached_unorm_to_float(value[0]),
                  encoding::cached_unorm_to_float(value[1]), 0.f);
}

float Byte2_unorm::at_1(int32_t x, int32_t y) const noexcept {
    auto value = image_.load(x, y);
    return encoding::cached_unorm_to_float(value[0]);
}

float2 Byte2_unorm::at_2(int32_t x, int32_t y) const noexcept {
    auto value = image_.load(x, y);
    return float2(encoding::cached_unorm_to_float(value[0]),
                  encoding::cached_unorm_to_float(value[1]));
}

float3 Byte2_unorm::at_3(int32_t x, int32_t y) const noexcept {
    auto value = image_.load(x, y);
    return float3(encoding::cached_unorm_to_float(value[0]),
                  encoding::cached_unorm_to_float(value[1]), 0.f);
}

float4 Byte2_unorm::at_4(int32_t x, int32_t y) const noexcept {
    auto value = image_.load(x, y);
    return float4(encoding::cached_unorm_to_float(value[0]),
                  encoding::cached_unorm_to_float(value[1]), 0.f, 1.f);
}

void Byte2_unorm::gather_1(int4 const& xy_xy1, float c[4]) const noexcept {
    byte2 v[4];
    image_.gather(xy_xy1, v);

    c[0] = encoding::cached_unorm_to_float(v[0][0]);
    c[1] = encoding::cached_unorm_to_float(v[1][0]);
    c[2] = encoding::cached_unorm_to_float(v[2][0]);
    c[3] = encoding::cached_unorm_to_float(v[3][0]);
}

void Byte2_unorm::gather_2(int4 const& xy_xy1, float2 c[4]) const noexcept {
    byte2 v[4];
    image_.gather(xy_xy1, v);

    encoding::cached_unorm_to_float(v, c);
}

void Byte2_unorm::gather_3(int4 const& xy_xy1, float3 c[4]) const noexcept {
    byte2 v[4];
    image_.gather(xy_xy1, v);

    c[0] = float3(encoding::cached_unorm_to_float(v[0][0]),
                  encoding::cached_unorm_to_float(v[0][1]), 0.f);

    c[1] = float3(encoding::cached_unorm_to_float(v[1][0]),
                  encoding::cached_unorm_to_float(v[1][1]), 0.f);

    c[2] = float3(encoding::cached_unorm_to_float(v[2][0]),
                  encoding::cached_unorm_to_float(v[2][1]), 0.f);

    c[3] = float3(encoding::cached_unorm_to_float(v[3][0]),
                  encoding::cached_unorm_to_float(v[3][1]), 0.f);
}

float Byte2_unorm::at_element_1(int32_t x, int32_t y, int32_t element) const noexcept {
    auto value = image_.load_element(x, y, element);
    return encoding::cached_unorm_to_float(value[0]);
}

float2 Byte2_unorm::at_element_2(int32_t x, int32_t y, int32_t element) const noexcept {
    auto value = image_.load_element(x, y, element);
    return float2(encoding::cached_unorm_to_float(value[0]),
                  encoding::cached_unorm_to_float(value[1]));
}

float3 Byte2_unorm::at_element_3(int32_t x, int32_t y, int32_t element) const noexcept {
    auto value = image_.load_element(x, y, element);
    return float3(encoding::cached_unorm_to_float(value[0]),
                  encoding::cached_unorm_to_float(value[1]), 0.f);
}

float Byte2_unorm::at_1(int32_t x, int32_t y, int32_t z) const noexcept {
    auto value = image_.load(x, y, z);
    return encoding::cached_unorm_to_float(value[0]);
}

float2 Byte2_unorm::at_2(int32_t x, int32_t y, int32_t z) const noexcept {
    auto value = image_.load(x, y, z);
    return float2(encoding::cached_unorm_to_float(value[0]),
                  encoding::cached_unorm_to_float(value[1]));
}

float3 Byte2_unorm::at_3(int32_t x, int32_t y, int32_t z) const noexcept {
    auto value = image_.load(x, y, z);
    return float3(encoding::cached_unorm_to_float(value[0]),
                  encoding::cached_unorm_to_float(value[1]), 0.f);
}

float4 Byte2_unorm::at_4(int32_t x, int32_t y, int32_t z) const noexcept {
    auto value = image_.load(x, y, z);
    return float4(encoding::cached_unorm_to_float(value[0]),
                  encoding::cached_unorm_to_float(value[1]), 0.f, 1.f);
}

}  // namespace image::texture
