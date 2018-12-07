#include "texture_byte_3_snorm.hpp"
#include "base/math/vector4.inl"
#include "image/typed_image.hpp"
#include "texture_encoding.hpp"

namespace image::texture {

Byte3_snorm::Byte3_snorm(std::shared_ptr<Image> const& image) noexcept
    : Texture(image), image_(*static_cast<const Byte3*>(image.get())) {}

float Byte3_snorm::at_1(int32_t i) const noexcept {
    auto value = image_.load(i);
    return encoding::cached_snorm_to_float(value[0]);
}

float3 Byte3_snorm::at_3(int32_t i) const noexcept {
    auto value = image_.load(i);
    return float3(encoding::cached_snorm_to_float(value[0]),
                  encoding::cached_snorm_to_float(value[1]),
                  encoding::cached_snorm_to_float(value[2]));
}

float Byte3_snorm::at_1(int32_t x, int32_t y) const noexcept {
    auto value = image_.load(x, y);
    return encoding::cached_snorm_to_float(value[0]);
}

float2 Byte3_snorm::at_2(int32_t x, int32_t y) const noexcept {
    auto value = image_.load(x, y);
    return float2(encoding::cached_snorm_to_float(value[0]),
                  encoding::cached_snorm_to_float(value[1]));
}

float3 Byte3_snorm::at_3(int32_t x, int32_t y) const noexcept {
    auto value = image_.load(x, y);
    return float3(encoding::cached_snorm_to_float(value[0]),
                  encoding::cached_snorm_to_float(value[1]),
                  encoding::cached_snorm_to_float(value[2]));
}

void Byte3_snorm::gather_1(int4 const& xy_xy1, float c[4]) const noexcept {
    byte3 v[4];
    image_.gather(xy_xy1, v);

    c[0] = encoding::cached_snorm_to_float(v[0][0]);
    c[1] = encoding::cached_snorm_to_float(v[1][0]);
    c[2] = encoding::cached_snorm_to_float(v[2][0]);
    c[3] = encoding::cached_snorm_to_float(v[3][0]);
}

void Byte3_snorm::gather_2(int4 const& xy_xy1, float2 c[4]) const noexcept {
    byte3 v[4];
    image_.gather(xy_xy1, v);

    c[0] = float2(encoding::cached_snorm_to_float(v[0][0]),
                  encoding::cached_snorm_to_float(v[0][1]));

    c[1] = float2(encoding::cached_snorm_to_float(v[1][0]),
                  encoding::cached_snorm_to_float(v[1][1]));

    c[2] = float2(encoding::cached_snorm_to_float(v[2][0]),
                  encoding::cached_snorm_to_float(v[2][1]));

    c[3] = float2(encoding::cached_snorm_to_float(v[3][0]),
                  encoding::cached_snorm_to_float(v[3][1]));
}

void Byte3_snorm::gather_3(int4 const& xy_xy1, float3 c[4]) const noexcept {
    byte3 v[4];
    image_.gather(xy_xy1, v);

    encoding::cached_snorm_to_float(v, c);
}

float Byte3_snorm::at_element_1(int32_t x, int32_t y, int32_t element) const noexcept {
    auto value = image_.load_element(x, y, element);
    return encoding::cached_snorm_to_float(value[0]);
}

float2 Byte3_snorm::at_element_2(int32_t x, int32_t y, int32_t element) const noexcept {
    auto value = image_.load_element(x, y, element);
    return float2(encoding::cached_snorm_to_float(value[0]),
                  encoding::cached_snorm_to_float(value[1]));
}

float3 Byte3_snorm::at_element_3(int32_t x, int32_t y, int32_t element) const noexcept {
    auto value = image_.load_element(x, y, element);
    return float3(encoding::cached_snorm_to_float(value[0]),
                  encoding::cached_snorm_to_float(value[1]),
                  encoding::cached_snorm_to_float(value[2]));
}

float Byte3_snorm::at_1(int32_t x, int32_t y, int32_t z) const noexcept {
    auto value = image_.load(x, y, z);
    return encoding::cached_snorm_to_float(value[0]);
}

float2 Byte3_snorm::at_2(int32_t x, int32_t y, int32_t z) const noexcept {
    auto value = image_.load(x, y, z);
    return float2(encoding::cached_snorm_to_float(value[0]),
                  encoding::cached_snorm_to_float(value[1]));
}

float3 Byte3_snorm::at_3(int32_t x, int32_t y, int32_t z) const noexcept {
    auto value = image_.load(x, y, z);
    return float3(encoding::cached_snorm_to_float(value[0]),
                  encoding::cached_snorm_to_float(value[1]),
                  encoding::cached_snorm_to_float(value[2]));
}

}  // namespace image::texture
