#include "texture_byte3_srgb.hpp"
#include "base/math/vector4.inl"
#include "base/spectrum/aces.hpp"
#include "image/typed_image.hpp"
#include "texture_encoding.inl"

namespace image::texture {

Byte3_sRGB::Byte3_sRGB(Byte3 const& image) : image_(image) {}

Byte3 const& Byte3_sRGB::image() const {
    return image_;
}

float Byte3_sRGB::at_1(int32_t x, int32_t y) const {
    auto const value = image_.load(x, y);
    return encoding::cached_srgb_to_float(value[0]);
}

float2 Byte3_sRGB::at_2(int32_t x, int32_t y) const {
    auto const value = image_.load(x, y);
    return encoding::cached_srgb_to_float(value.xy());
}

float3 Byte3_sRGB::at_3(int32_t x, int32_t y) const {
    auto const value = image_.load(x, y);
#ifdef SU_ACESCG
    return spectrum::linear_sRGB_to_AP1(encoding::cached_srgb_to_float(value));
#else
    return encoding::cached_srgb_to_float(value);
#endif
}

float4 Byte3_sRGB::at_4(int32_t x, int32_t y) const {
    auto const value = image_.load(x, y);
#ifdef SU_ACESCG
    return float4(spectrum::linear_sRGB_to_AP1(encoding::cached_srgb_to_float(value)));
#else
    return float4(encoding::cached_srgb_to_float(value));
#endif
}

void Byte3_sRGB::gather_1(int4 const& xy_xy1, float c[4]) const {
    byte3 v[4];
    image_.gather(xy_xy1, v);

    c[0] = encoding::cached_srgb_to_float(v[0][0]);
    c[1] = encoding::cached_srgb_to_float(v[1][0]);
    c[2] = encoding::cached_srgb_to_float(v[2][0]);
    c[3] = encoding::cached_srgb_to_float(v[3][0]);
}

void Byte3_sRGB::gather_2(int4 const& xy_xy1, float2 c[4]) const {
    byte3 v[4];
    image_.gather(xy_xy1, v);

    c[0] = encoding::cached_srgb_to_float(v[0].xy());
    c[1] = encoding::cached_srgb_to_float(v[1].xy());
    c[2] = encoding::cached_srgb_to_float(v[2].xy());
    c[3] = encoding::cached_srgb_to_float(v[3].xy());
}

void Byte3_sRGB::gather_3(int4 const& xy_xy1, float3 c[4]) const {
    byte3 v[4];
    image_.gather(xy_xy1, v);

#ifdef SU_ACESCG
    c[0] = spectrum::linear_sRGB_to_AP1(encoding::cached_srgb_to_float(v[0]));
    c[1] = spectrum::linear_sRGB_to_AP1(encoding::cached_srgb_to_float(v[1]));
    c[2] = spectrum::linear_sRGB_to_AP1(encoding::cached_srgb_to_float(v[2]));
    c[3] = spectrum::linear_sRGB_to_AP1(encoding::cached_srgb_to_float(v[3]));
#else
    c[0] = encoding::cached_srgb_to_float(v[0]);
    c[1] = encoding::cached_srgb_to_float(v[1]);
    c[2] = encoding::cached_srgb_to_float(v[2]);
    c[3] = encoding::cached_srgb_to_float(v[3]);
#endif
}

float Byte3_sRGB::at_element_1(int32_t x, int32_t y, int32_t element) const {
    auto const value = image_.load_element(x, y, element);
    return encoding::cached_srgb_to_float(value[0]);
}

float2 Byte3_sRGB::at_element_2(int32_t x, int32_t y, int32_t element) const {
    auto const value = image_.load_element(x, y, element);
    return float2(encoding::cached_srgb_to_float(value[0]),
                  encoding::cached_srgb_to_float(value[1]));
}

float3 Byte3_sRGB::at_element_3(int32_t x, int32_t y, int32_t element) const {
    auto const value = image_.load_element(x, y, element);
#ifdef SU_ACESCG
    return spectrum::linear_sRGB_to_AP1(encoding::cached_srgb_to_float(value));
#else
    return encoding::cached_srgb_to_float(value);
#endif
}

float Byte3_sRGB::at_1(int32_t x, int32_t y, int32_t z) const {
    auto const value = image_.load(x, y, z);
    return encoding::cached_srgb_to_float(value[0]);
}

float2 Byte3_sRGB::at_2(int32_t x, int32_t y, int32_t z) const {
    auto const value = image_.load(x, y, z);
    return float2(encoding::cached_srgb_to_float(value[0]),
                  encoding::cached_srgb_to_float(value[1]));
}

float3 Byte3_sRGB::at_3(int32_t x, int32_t y, int32_t z) const {
    auto const value = image_.load(x, y, z);
#ifdef SU_ACESCG
    return spectrum::linear_sRGB_to_AP1(encoding::cached_srgb_to_float(value));
#else
    return encoding::cached_srgb_to_float(value);
#endif
}

float4 Byte3_sRGB::at_4(int32_t x, int32_t y, int32_t z) const {
    auto const value = image_.load(x, y, z);
#ifdef SU_ACESCG
    return float4(spectrum::linear_sRGB_to_AP1(encoding::cached_srgb_to_float(value)));
#else
    return float4(encoding::cached_srgb_to_float(value));
#endif
}

}  // namespace image::texture
