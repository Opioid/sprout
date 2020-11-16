#include "texture_byte4_srgb.hpp"
#include "base/math/vector4.inl"
#include "base/spectrum/aces.hpp"
#include "image/typed_image.hpp"
#include "texture_encoding.inl"

namespace image::texture {

Byte4_sRGB::Byte4_sRGB(Byte4 const& image) : image_(image) {}

Byte4 const& Byte4_sRGB::image() const {
    return image_;
}

float Byte4_sRGB::at_1(int32_t x, int32_t y) const {
    auto const value = image_.at(x, y);
    return encoding::cached_srgb_to_float(value[0]);
}

float2 Byte4_sRGB::at_2(int32_t x, int32_t y) const {
    auto const value = image_.at(x, y);
    return encoding::cached_srgb_to_float(value.xy());
}

float3 Byte4_sRGB::at_3(int32_t x, int32_t y) const {
    auto const value = image_.at(x, y);
    return encoding::cached_srgb_to_float(value.xyz());
}

float4 Byte4_sRGB::at_4(int32_t x, int32_t y) const {
    auto const value = image_.at(x, y);
    return encoding::cached_srgb_to_float(value);
}

void Byte4_sRGB::gather_1(int4_p xy_xy1, float c[4]) const {
    byte4 v[4];
    image_.gather(xy_xy1, v);

#ifdef SU_ACESCG
    c[0] = spectrum::sRGB_to_AP1(encoding::cached_srgb_to_float(v[0].xyz()))[0];
    c[1] = spectrum::sRGB_to_AP1(encoding::cached_srgb_to_float(v[1]).xyz())[0];
    c[2] = spectrum::sRGB_to_AP1(encoding::cached_srgb_to_float(v[2]).xyz())[0];
    c[3] = spectrum::sRGB_to_AP1(encoding::cached_srgb_to_float(v[3]).xyz())[0];
#else
    c[0] = encoding::cached_srgb_to_float(v[0][0]);
    c[1] = encoding::cached_srgb_to_float(v[1][0]);
    c[2] = encoding::cached_srgb_to_float(v[2][0]);
    c[3] = encoding::cached_srgb_to_float(v[3][0]);
#endif
}

void Byte4_sRGB::gather_2(int4_p xy_xy1, float2 c[4]) const {
    byte4 v[4];
    image_.gather(xy_xy1, v);

#ifdef SU_ACESCG
    c[0] = spectrum::sRGB_to_AP1(encoding::cached_srgb_to_float(v[0]).xyz()).xy();
    c[1] = spectrum::sRGB_to_AP1(encoding::cached_srgb_to_float(v[1]).xyz()).xy();
    c[2] = spectrum::sRGB_to_AP1(encoding::cached_srgb_to_float(v[2]).xyz()).xy();
    c[3] = spectrum::sRGB_to_AP1(encoding::cached_srgb_to_float(v[3]).xyz()).xy();
#else
    c[0] = encoding::cached_srgb_to_float(v[0].xy());
    c[1] = encoding::cached_srgb_to_float(v[1].xy());
    c[2] = encoding::cached_srgb_to_float(v[2].xy());
    c[3] = encoding::cached_srgb_to_float(v[3].xy());
#endif
}

void Byte4_sRGB::gather_3(int4_p xy_xy1, float3 c[4]) const {
    byte4 v[4];
    image_.gather(xy_xy1, v);

#ifdef SU_ACESCG
    c[0] = spectrum::sRGB_to_AP1(encoding::cached_srgb_to_float(v[0].xyz()));
    c[1] = spectrum::sRGB_to_AP1(encoding::cached_srgb_to_float(v[1].xyz()));
    c[2] = spectrum::sRGB_to_AP1(encoding::cached_srgb_to_float(v[2].xyz()));
    c[3] = spectrum::sRGB_to_AP1(encoding::cached_srgb_to_float(v[3].xyz()));
#else
    c[0] = encoding::cached_srgb_to_float(v[0].xyz());
    c[1] = encoding::cached_srgb_to_float(v[1].xyz());
    c[2] = encoding::cached_srgb_to_float(v[2].xyz());
    c[3] = encoding::cached_srgb_to_float(v[3].xyz());
#endif
}

float Byte4_sRGB::at_element_1(int32_t x, int32_t y, int32_t element) const {
    auto const value = image_.at_element(x, y, element);
    return encoding::cached_srgb_to_float(value[0]);
}

float2 Byte4_sRGB::at_element_2(int32_t x, int32_t y, int32_t element) const {
    auto const value = image_.at_element(x, y, element);
    return float2(encoding::cached_srgb_to_float(value[0]),
                  encoding::cached_srgb_to_float(value[1]));
}

float3 Byte4_sRGB::at_element_3(int32_t x, int32_t y, int32_t element) const {
    auto const value = image_.at_element(x, y, element);
    return encoding::cached_srgb_to_float(value.xyz());
}

float Byte4_sRGB::at_1(int32_t x, int32_t y, int32_t z) const {
    auto const value = image_.at(x, y, z);
    return encoding::cached_srgb_to_float(value[0]);
}

float2 Byte4_sRGB::at_2(int32_t x, int32_t y, int32_t z) const {
    auto const value = image_.at(x, y, z);
    return float2(encoding::cached_srgb_to_float(value[0]),
                  encoding::cached_srgb_to_float(value[1]));
}

float3 Byte4_sRGB::at_3(int32_t x, int32_t y, int32_t z) const {
    auto const value = image_.at(x, y, z);
    return encoding::cached_srgb_to_float(value.xyz());
}

float4 Byte4_sRGB::at_4(int32_t x, int32_t y, int32_t z) const {
    auto const value = image_.at(x, y, z);
    return encoding::cached_srgb_to_float(value);
}

void Byte4_sRGB::gather_1(int3_p xyz, int3_p xyz1, float c[8]) const {
    byte4 v[8];
    image_.gather(xyz, xyz1, v);

#ifdef SU_ACESCG
    c[0] = spectrum::sRGB_to_AP1(encoding::cached_srgb_to_float(v[0]).xyz())[0];
    c[1] = spectrum::sRGB_to_AP1(encoding::cached_srgb_to_float(v[1]).xyz())[0];
    c[2] = spectrum::sRGB_to_AP1(encoding::cached_srgb_to_float(v[2]).xyz())[0];
    c[3] = spectrum::sRGB_to_AP1(encoding::cached_srgb_to_float(v[3]).xyz())[0];
    c[4] = spectrum::sRGB_to_AP1(encoding::cached_srgb_to_float(v[4]).xyz())[0];
    c[5] = spectrum::sRGB_to_AP1(encoding::cached_srgb_to_float(v[5]).xyz())[0];
    c[6] = spectrum::sRGB_to_AP1(encoding::cached_srgb_to_float(v[6]).xyz())[0];
    c[7] = spectrum::sRGB_to_AP1(encoding::cached_srgb_to_float(v[7]).xyz())[0];
#else
    c[0] = encoding::cached_srgb_to_float(v[0][0]);
    c[1] = encoding::cached_srgb_to_float(v[1][0]);
    c[2] = encoding::cached_srgb_to_float(v[2][0]);
    c[3] = encoding::cached_srgb_to_float(v[3][0]);
    c[4] = encoding::cached_srgb_to_float(v[4][0]);
    c[5] = encoding::cached_srgb_to_float(v[5][0]);
    c[6] = encoding::cached_srgb_to_float(v[6][0]);
    c[7] = encoding::cached_srgb_to_float(v[7][0]);
#endif
}

void Byte4_sRGB::gather_2(int3_p xyz, int3_p xyz1, float2 c[8]) const {
    byte4 v[8];
    image_.gather(xyz, xyz1, v);

#ifdef SU_ACESCG
    c[0] = spectrum::sRGB_to_AP1(encoding::cached_srgb_to_float(v[0]).xyz()).xy();
    c[1] = spectrum::sRGB_to_AP1(encoding::cached_srgb_to_float(v[1]).xyz()).xy();
    c[2] = spectrum::sRGB_to_AP1(encoding::cached_srgb_to_float(v[2]).xyz()).xy();
    c[3] = spectrum::sRGB_to_AP1(encoding::cached_srgb_to_float(v[3]).xyz()).xy();
    c[4] = spectrum::sRGB_to_AP1(encoding::cached_srgb_to_float(v[4]).xyz()).xy();
    c[5] = spectrum::sRGB_to_AP1(encoding::cached_srgb_to_float(v[5]).xyz()).xy();
    c[6] = spectrum::sRGB_to_AP1(encoding::cached_srgb_to_float(v[6]).xyz()).xy();
    c[7] = spectrum::sRGB_to_AP1(encoding::cached_srgb_to_float(v[7]).xyz()).xy();
#else
    c[0] = encoding::cached_srgb_to_float(v[0].xy());
    c[1] = encoding::cached_srgb_to_float(v[1].xy());
    c[2] = encoding::cached_srgb_to_float(v[2].xy());
    c[3] = encoding::cached_srgb_to_float(v[3].xy());
    c[4] = encoding::cached_srgb_to_float(v[4].xy());
    c[5] = encoding::cached_srgb_to_float(v[5].xy());
    c[6] = encoding::cached_srgb_to_float(v[6].xy());
    c[7] = encoding::cached_srgb_to_float(v[7].xy());
#endif
}

}  // namespace image::texture
