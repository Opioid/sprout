#include "texture_half3.hpp"
#include "base/math/half.inl"
#include "base/math/vector4.inl"
#include "image/typed_image.hpp"

namespace image::texture {

Half3::Half3(image::Short3 const& image)
    : description_(image.description()),
      data_(image.data()),
      dimensions_(image.description().dimensions()) {}

Description const& Half3::description() const {
    return description_;
}

float Half3::at_1(int32_t x, int32_t y) const {
    int32_t const i = y * dimensions_[0] + x;
    return half_to_float(data_[i][0]);
}

float2 Half3::at_2(int32_t x, int32_t y) const {
    int32_t const i = y * dimensions_[0] + x;
    return half_to_float(data_[i].xy());
}

float3 Half3::at_3(int32_t x, int32_t y) const {
    int32_t const i = y * dimensions_[0] + x;
    return half_to_float(data_[i]);
}

float4 Half3::at_4(int32_t x, int32_t y) const {
    int32_t const i = y * dimensions_[0] + x;
    return float4(half_to_float(data_[i]), 1.f);
}

void Half3::gather_1(int4_p xy_xy1, float c[4]) const {
    int32_t const width = dimensions_[0];

    int32_t const y0 = width * xy_xy1[1];

    uint16_t const v0 = data_[y0 + xy_xy1[0]][0];
    uint16_t const v1 = data_[y0 + xy_xy1[2]][0];

    int32_t const y1 = width * xy_xy1[3];

    uint16_t const v2 = data_[y1 + xy_xy1[0]][0];
    uint16_t const v3 = data_[y1 + xy_xy1[2]][0];

    c[0] = half_to_float(v0);
    c[1] = half_to_float(v1);
    c[2] = half_to_float(v2);
    c[3] = half_to_float(v3);
}

void Half3::gather_2(int4_p xy_xy1, float2 c[4]) const {
    int32_t const width = dimensions_[0];

    int32_t const y0 = width * xy_xy1[1];

    ushort2 const v0 = data_[y0 + xy_xy1[0]].xy();
    ushort2 const v1 = data_[y0 + xy_xy1[2]].xy();

    int32_t const y1 = width * xy_xy1[3];

    ushort2 const v2 = data_[y1 + xy_xy1[0]].xy();
    ushort2 const v3 = data_[y1 + xy_xy1[2]].xy();

    c[0] = half_to_float(v0);
    c[1] = half_to_float(v1);
    c[2] = half_to_float(v2);
    c[3] = half_to_float(v3);
}

void Half3::gather_3(int4_p xy_xy1, float3 c[4]) const {
    int32_t const width = dimensions_[0];

    int32_t const y0 = width * xy_xy1[1];

    ushort3 const v0 = data_[y0 + xy_xy1[0]];
    ushort3 const v1 = data_[y0 + xy_xy1[2]];

    int32_t const y1 = width * xy_xy1[3];

    ushort3 const v2 = data_[y1 + xy_xy1[0]];
    ushort3 const v3 = data_[y1 + xy_xy1[2]];

    c[0] = half_to_float(v0);
    c[1] = half_to_float(v1);
    c[2] = half_to_float(v2);
    c[3] = half_to_float(v3);
}

float Half3::at_element_1(int32_t x, int32_t y, int32_t element) const {
    int32_t const i = (element * dimensions_[1] + y) * dimensions_[0] + x;

    return half_to_float(data_[i][0]);
}

float2 Half3::at_element_2(int32_t x, int32_t y, int32_t element) const {
    int32_t const i = (element * dimensions_[1] + y) * dimensions_[0] + x;

    return half_to_float(data_[i].xy());
}

float3 Half3::at_element_3(int32_t x, int32_t y, int32_t element) const {
    int32_t const i = (element * dimensions_[1] + y) * dimensions_[0] + x;

    return half_to_float(data_[i]);
}

float Half3::at_1(int32_t x, int32_t y, int32_t z) const {
    int64_t const i = (int64_t(z) * int64_t(dimensions_[1]) + int64_t(y)) *
                          int64_t(dimensions_[0]) +
                      int64_t(x);

    return half_to_float(data_[i][0]);
}

float2 Half3::at_2(int32_t x, int32_t y, int32_t z) const {
    int64_t const i = (int64_t(z) * int64_t(dimensions_[1]) + int64_t(y)) *
                          int64_t(dimensions_[0]) +
                      int64_t(x);

    return half_to_float(data_[i].xy());
}

float3 Half3::at_3(int32_t x, int32_t y, int32_t z) const {
    int64_t const i = (int64_t(z) * int64_t(dimensions_[1]) + int64_t(y)) *
                          int64_t(dimensions_[0]) +
                      int64_t(x);

    return half_to_float(data_[i]);
}

float4 Half3::at_4(int32_t x, int32_t y, int32_t z) const {
    int64_t const i = (int64_t(z) * int64_t(dimensions_[1]) + int64_t(y)) *
                          int64_t(dimensions_[0]) +
                      int64_t(x);

    return float4(half_to_float(data_[i]), 1.f);
}

void Half3::gather_1(int3_p xyz, int3_p xyz1, float c[8]) const {
    int64_t const w = int64_t(dimensions_[0]);
    int64_t const h = int64_t(dimensions_[1]);

    int64_t const x = int64_t(xyz[0]);
    int64_t const y = int64_t(xyz[1]);
    int64_t const z = int64_t(xyz[2]);

    int64_t const x1 = int64_t(xyz1[0]);
    int64_t const y1 = int64_t(xyz1[1]);
    int64_t const z1 = int64_t(xyz1[2]);

    int64_t const d = z * h;

    int64_t const  c0 = (d + y) * w + x;
    uint16_t const v0 = data_[c0][0];

    int64_t const  c1 = (d + y) * w + x1;
    uint16_t const v1 = data_[c1][0];

    int64_t const  c2 = (d + y1) * w + x;
    uint16_t const v2 = data_[c2][0];

    int64_t const  c3 = (d + y1) * w + x1;
    uint16_t const v3 = data_[c3][0];

    int64_t const d1 = z1 * h;

    int64_t const  c4 = (d1 + y) * w + x;
    uint16_t const v4 = data_[c4][0];

    int64_t const  c5 = (d1 + y) * w + x1;
    uint16_t const v5 = data_[c5][0];

    int64_t const  c6 = (d1 + y1) * w + x;
    uint16_t const v6 = data_[c6][0];

    int64_t const  c7 = (d1 + y1) * w + x1;
    uint16_t const v7 = data_[c7][0];

    c[0] = half_to_float(v0);
    c[1] = half_to_float(v1);
    c[2] = half_to_float(v2);
    c[3] = half_to_float(v3);
    c[4] = half_to_float(v4);
    c[5] = half_to_float(v5);
    c[6] = half_to_float(v6);
    c[7] = half_to_float(v7);
}

void Half3::gather_2(int3_p xyz, int3_p xyz1, float2 c[8]) const {
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
    ushort2 const v0 = data_[c0].xy();

    int64_t const c1 = (d + y) * w + x1;
    ushort2 const v1 = data_[c1].xy();

    int64_t const c2 = (d + y1) * w + x;
    ushort2 const v2 = data_[c2].xy();

    int64_t const c3 = (d + y1) * w + x1;
    ushort2 const v3 = data_[c3].xy();

    int64_t const d1 = z1 * h;

    int64_t const c4 = (d1 + y) * w + x;
    ushort2 const v4 = data_[c4].xy();

    int64_t const c5 = (d1 + y) * w + x1;
    ushort2 const v5 = data_[c5].xy();

    int64_t const c6 = (d1 + y1) * w + x;
    ushort2 const v6 = data_[c6].xy();

    int64_t const c7 = (d1 + y1) * w + x1;
    ushort2 const v7 = data_[c7].xy();

    c[0] = half_to_float(v0);
    c[1] = half_to_float(v1);
    c[2] = half_to_float(v2);
    c[3] = half_to_float(v3);
    c[4] = half_to_float(v4);
    c[5] = half_to_float(v5);
    c[6] = half_to_float(v6);
    c[7] = half_to_float(v7);
}

}  // namespace image::texture
