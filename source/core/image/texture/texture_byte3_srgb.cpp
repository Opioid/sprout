#include "texture_byte3_srgb.hpp"
#include "base/math/vector4.inl"
#include "base/spectrum/aces.hpp"
#include "image/typed_image.hpp"
#include "texture_encoding.inl"

namespace image::texture {

Byte3_sRGB::Byte3_sRGB(Byte3 const& image)
    : description_(image.description()),
      data_(image.data()),
      dimensions_(image.description().dimensions()) {}

Description const& Byte3_sRGB::description() const {
    return description_;
}

float Byte3_sRGB::at_1(int32_t x, int32_t y) const {
    int32_t const i     = y * dimensions_[0] + x;
    auto const    value = data_[i];
    return encoding::cached_srgb_to_float(value[0]);
}

float2 Byte3_sRGB::at_2(int32_t x, int32_t y) const {
    int32_t const i     = y * dimensions_[0] + x;
    auto const    value = data_[i];
    return encoding::cached_srgb_to_float(value.xy());
}

float3 Byte3_sRGB::at_3(int32_t x, int32_t y) const {
    int32_t const i     = y * dimensions_[0] + x;
    auto const    value = data_[i];
#ifdef SU_ACESCG
    return spectrum::sRGB_to_AP1(encoding::cached_srgb_to_float(value));
#else
    return encoding::cached_srgb_to_float(value);
#endif
}

float4 Byte3_sRGB::at_4(int32_t x, int32_t y) const {
    int32_t const i     = y * dimensions_[0] + x;
    auto const    value = data_[i];
#ifdef SU_ACESCG
    return float4(spectrum::sRGB_to_AP1(encoding::cached_srgb_to_float(value)));
#else
    return float4(encoding::cached_srgb_to_float(value));
#endif
}

void Byte3_sRGB::gather_1(int4_p xy_xy1, float c[4]) const {
    int32_t const width = dimensions_[0];

    int32_t const y0 = width * xy_xy1[1];

    byte3 const v0 = data_[y0 + xy_xy1[0]];
    byte3 const v1 = data_[y0 + xy_xy1[2]];

    int32_t const y1 = width * xy_xy1[3];

    byte3 const v2 = data_[y1 + xy_xy1[0]];
    byte3 const v3 = data_[y1 + xy_xy1[2]];

#ifdef SU_ACESCG
    c[0] = spectrum::sRGB_to_AP1(encoding::cached_srgb_to_float(v0))[0];
    c[1] = spectrum::sRGB_to_AP1(encoding::cached_srgb_to_float(v1))[0];
    c[2] = spectrum::sRGB_to_AP1(encoding::cached_srgb_to_float(v2))[0];
    c[3] = spectrum::sRGB_to_AP1(encoding::cached_srgb_to_float(v3))[0];
#else
    c[0] = encoding::cached_srgb_to_float(v[0][0]);
    c[1] = encoding::cached_srgb_to_float(v[1][0]);
    c[2] = encoding::cached_srgb_to_float(v[2][0]);
    c[3] = encoding::cached_srgb_to_float(v[3][0]);
#endif
}

void Byte3_sRGB::gather_2(int4_p xy_xy1, float2 c[4]) const {
    int32_t const width = dimensions_[0];

    int32_t const y0 = width * xy_xy1[1];

    byte3 const v0 = data_[y0 + xy_xy1[0]];
    byte3 const v1 = data_[y0 + xy_xy1[2]];

    int32_t const y1 = width * xy_xy1[3];

    byte3 const v2 = data_[y1 + xy_xy1[0]];
    byte3 const v3 = data_[y1 + xy_xy1[2]];

#ifdef SU_ACESCG
    c[0] = spectrum::sRGB_to_AP1(encoding::cached_srgb_to_float(v0)).xy();
    c[1] = spectrum::sRGB_to_AP1(encoding::cached_srgb_to_float(v1)).xy();
    c[2] = spectrum::sRGB_to_AP1(encoding::cached_srgb_to_float(v2)).xy();
    c[3] = spectrum::sRGB_to_AP1(encoding::cached_srgb_to_float(v3)).xy();
#else
    c[0] = encoding::cached_srgb_to_float(v[0].xy());
    c[1] = encoding::cached_srgb_to_float(v[1].xy());
    c[2] = encoding::cached_srgb_to_float(v[2].xy());
    c[3] = encoding::cached_srgb_to_float(v[3].xy());
#endif
}

void Byte3_sRGB::gather_3(int4_p xy_xy1, float3 c[4]) const {
    int32_t const width = dimensions_[0];

    int32_t const y0 = width * xy_xy1[1];

    byte3 const v0 = data_[y0 + xy_xy1[0]];
    byte3 const v1 = data_[y0 + xy_xy1[2]];

    int32_t const y1 = width * xy_xy1[3];

    byte3 const v2 = data_[y1 + xy_xy1[0]];
    byte3 const v3 = data_[y1 + xy_xy1[2]];

#ifdef SU_ACESCG
    c[0] = spectrum::sRGB_to_AP1(encoding::cached_srgb_to_float(v0));
    c[1] = spectrum::sRGB_to_AP1(encoding::cached_srgb_to_float(v1));
    c[2] = spectrum::sRGB_to_AP1(encoding::cached_srgb_to_float(v2));
    c[3] = spectrum::sRGB_to_AP1(encoding::cached_srgb_to_float(v3));
#else
    c[0] = encoding::cached_srgb_to_float(v0);
    c[1] = encoding::cached_srgb_to_float(v1);
    c[2] = encoding::cached_srgb_to_float(v2);
    c[3] = encoding::cached_srgb_to_float(v3);
#endif
}

float Byte3_sRGB::at_element_1(int32_t x, int32_t y, int32_t element) const {
    int32_t const i     = (element * dimensions_[1] + y) * dimensions_[0] + x;
    auto const    value = data_[i];
    return encoding::cached_srgb_to_float(value[0]);
}

float2 Byte3_sRGB::at_element_2(int32_t x, int32_t y, int32_t element) const {
    int32_t const i     = (element * dimensions_[1] + y) * dimensions_[0] + x;
    auto const    value = data_[i];
    return float2(encoding::cached_srgb_to_float(value[0]),
                  encoding::cached_srgb_to_float(value[1]));
}

float3 Byte3_sRGB::at_element_3(int32_t x, int32_t y, int32_t element) const {
    int32_t const i     = (element * dimensions_[1] + y) * dimensions_[0] + x;
    auto const    value = data_[i];
#ifdef SU_ACESCG
    return spectrum::sRGB_to_AP1(encoding::cached_srgb_to_float(value));
#else
    return encoding::cached_srgb_to_float(value);
#endif
}

float Byte3_sRGB::at_1(int32_t x, int32_t y, int32_t z) const {
    int64_t const i = (int64_t(z) * int64_t(dimensions_[1]) + int64_t(y)) *
                          int64_t(dimensions_[0]) +
                      int64_t(x);
    auto const value = data_[i];
    return encoding::cached_srgb_to_float(value[0]);
}

float2 Byte3_sRGB::at_2(int32_t x, int32_t y, int32_t z) const {
    int64_t const i = (int64_t(z) * int64_t(dimensions_[1]) + int64_t(y)) *
                          int64_t(dimensions_[0]) +
                      int64_t(x);
    auto const value = data_[i];
    return float2(encoding::cached_srgb_to_float(value[0]),
                  encoding::cached_srgb_to_float(value[1]));
}

float3 Byte3_sRGB::at_3(int32_t x, int32_t y, int32_t z) const {
    int64_t const i = (int64_t(z) * int64_t(dimensions_[1]) + int64_t(y)) *
                          int64_t(dimensions_[0]) +
                      int64_t(x);
    auto const value = data_[i];
#ifdef SU_ACESCG
    return spectrum::sRGB_to_AP1(encoding::cached_srgb_to_float(value));
#else
    return encoding::cached_srgb_to_float(value);
#endif
}

float4 Byte3_sRGB::at_4(int32_t x, int32_t y, int32_t z) const {
    int64_t const i = (int64_t(z) * int64_t(dimensions_[1]) + int64_t(y)) *
                          int64_t(dimensions_[0]) +
                      int64_t(x);
    auto const value = data_[i];
#ifdef SU_ACESCG
    return float4(spectrum::sRGB_to_AP1(encoding::cached_srgb_to_float(value)));
#else
    return float4(encoding::cached_srgb_to_float(value));
#endif
}

void Byte3_sRGB::gather_1(int3_p xyz, int3_p xyz1, float c[8]) const {
    int64_t const w = int64_t(dimensions_[0]);
    int64_t const h = int64_t(dimensions_[1]);

    int64_t const x = int64_t(xyz[0]);
    int64_t const y = int64_t(xyz[1]);
    int64_t const z = int64_t(xyz[2]);

    int64_t const x1 = int64_t(xyz1[0]);
    int64_t const y1 = int64_t(xyz1[1]);
    int64_t const z1 = int64_t(xyz1[2]);

    int64_t const d = z * h;

    int64_t const c0 = (d + y) * w + x;
    byte3 const   v0 = data_[c0];

    int64_t const c1 = (d + y) * w + x1;
    byte3 const   v1 = data_[c1];

    int64_t const c2 = (d + y1) * w + x;
    byte3 const   v2 = data_[c2];

    int64_t const c3 = (d + y1) * w + x1;
    byte3 const   v3 = data_[c3];

    int64_t const d1 = z1 * h;

    int64_t const c4 = (d1 + y) * w + x;
    byte3 const   v4 = data_[c4];

    int64_t const c5 = (d1 + y) * w + x1;
    byte3 const   v5 = data_[c5];

    int64_t const c6 = (d1 + y1) * w + x;
    byte3 const   v6 = data_[c6];

    int64_t const c7 = (d1 + y1) * w + x1;
    byte3 const   v7 = data_[c7];

#ifdef SU_ACESCG
    c[0] = spectrum::sRGB_to_AP1(encoding::cached_srgb_to_float(v0))[0];
    c[1] = spectrum::sRGB_to_AP1(encoding::cached_srgb_to_float(v1))[0];
    c[2] = spectrum::sRGB_to_AP1(encoding::cached_srgb_to_float(v2))[0];
    c[3] = spectrum::sRGB_to_AP1(encoding::cached_srgb_to_float(v3))[0];
    c[4] = spectrum::sRGB_to_AP1(encoding::cached_srgb_to_float(v4))[0];
    c[5] = spectrum::sRGB_to_AP1(encoding::cached_srgb_to_float(v5))[0];
    c[6] = spectrum::sRGB_to_AP1(encoding::cached_srgb_to_float(v6))[0];
    c[7] = spectrum::sRGB_to_AP1(encoding::cached_srgb_to_float(v7))[0];
#else
    c[0] = encoding::cached_srgb_to_float(v0[0]);
    c[1] = encoding::cached_srgb_to_float(v1[0]);
    c[2] = encoding::cached_srgb_to_float(v2[0]);
    c[3] = encoding::cached_srgb_to_float(v3[0]);
    c[4] = encoding::cached_srgb_to_float(v4[0]);
    c[5] = encoding::cached_srgb_to_float(v5[0]);
    c[6] = encoding::cached_srgb_to_float(v6[0]);
    c[7] = encoding::cached_srgb_to_float(v7[0]);
#endif
}

void Byte3_sRGB::gather_2(int3_p xyz, int3_p xyz1, float2 c[8]) const {
    int64_t const w = int64_t(dimensions_[0]);
    int64_t const h = int64_t(dimensions_[1]);

    int64_t const x = int64_t(xyz[0]);
    int64_t const y = int64_t(xyz[1]);
    int64_t const z = int64_t(xyz[2]);

    int64_t const x1 = int64_t(xyz1[0]);
    int64_t const y1 = int64_t(xyz1[1]);
    int64_t const z1 = int64_t(xyz1[2]);

    int64_t const d = z * h;

    int64_t const c0 = (d + y) * w + x;
    byte3 const   v0 = data_[c0];

    int64_t const c1 = (d + y) * w + x1;
    byte3 const   v1 = data_[c1];

    int64_t const c2 = (d + y1) * w + x;
    byte3 const   v2 = data_[c2];

    int64_t const c3 = (d + y1) * w + x1;
    byte3 const   v3 = data_[c3];

    int64_t const d1 = z1 * h;

    int64_t const c4 = (d1 + y) * w + x;
    byte3 const   v4 = data_[c4];

    int64_t const c5 = (d1 + y) * w + x1;
    byte3 const   v5 = data_[c5];

    int64_t const c6 = (d1 + y1) * w + x;
    byte3 const   v6 = data_[c6];

    int64_t const c7 = (d1 + y1) * w + x1;
    byte3 const   v7 = data_[c7];

#ifdef SU_ACESCG
    c[0] = spectrum::sRGB_to_AP1(encoding::cached_srgb_to_float(v0)).xy();
    c[1] = spectrum::sRGB_to_AP1(encoding::cached_srgb_to_float(v1)).xy();
    c[2] = spectrum::sRGB_to_AP1(encoding::cached_srgb_to_float(v2)).xy();
    c[3] = spectrum::sRGB_to_AP1(encoding::cached_srgb_to_float(v3)).xy();
    c[4] = spectrum::sRGB_to_AP1(encoding::cached_srgb_to_float(v4)).xy();
    c[5] = spectrum::sRGB_to_AP1(encoding::cached_srgb_to_float(v5)).xy();
    c[6] = spectrum::sRGB_to_AP1(encoding::cached_srgb_to_float(v6)).xy();
    c[7] = spectrum::sRGB_to_AP1(encoding::cached_srgb_to_float(v7)).xy();
#else
    c[0] = encoding::cached_srgb_to_float(v0.xy());
    c[1] = encoding::cached_srgb_to_float(v1.xy());
    c[2] = encoding::cached_srgb_to_float(v2.xy());
    c[3] = encoding::cached_srgb_to_float(v3.xy());
    c[4] = encoding::cached_srgb_to_float(v4.xy());
    c[5] = encoding::cached_srgb_to_float(v5.xy());
    c[6] = encoding::cached_srgb_to_float(v6.xy());
    c[7] = encoding::cached_srgb_to_float(v7.xy());
#endif
}

}  // namespace image::texture
