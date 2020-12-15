#include "texture_float3.hpp"
#include "base/math/vector4.inl"
#include "image/typed_image.hpp"

namespace image::texture {

Float3::Float3(image::Float3 const& image)
    : description_(image.description()),
      data_(image.data()),
      dimensions_(image.description().dimensions()) {}

Description const& Float3::description() const {
    return description_;
}

float Float3::at_1(int32_t x, int32_t y) const {
    int32_t const i     = y * dimensions_[0] + x;
    auto const    value = data_[i];
    return value[0];
}

float2 Float3::at_2(int32_t x, int32_t y) const {
    int32_t const i     = y * dimensions_[0] + x;
    auto const    value = data_[i];
    return value.xy();
}

float3 Float3::at_3(int32_t x, int32_t y) const {
    int32_t const i     = y * dimensions_[0] + x;
    auto const    value = data_[i];
    return float3(value);
}

float4 Float3::at_4(int32_t x, int32_t y) const {
    int32_t const i     = y * dimensions_[0] + x;
    auto const    value = data_[i];
    return float4(value, 1.f);
}

void Float3::gather_1(int4_p xy_xy1, float c[4]) const {
    int32_t const width = dimensions_[0];

    int32_t const y0 = width * xy_xy1[1];

    c[0] = data_[y0 + xy_xy1[0]][0];
    c[1] = data_[y0 + xy_xy1[2]][0];

    int32_t const y1 = width * xy_xy1[3];

    c[2] = data_[y1 + xy_xy1[0]][0];
    c[3] = data_[y1 + xy_xy1[2]][0];
}

void Float3::gather_2(int4_p xy_xy1, float2 c[4]) const {
    int32_t const width = dimensions_[0];

    int32_t const y0 = width * xy_xy1[1];

    c[0] = data_[y0 + xy_xy1[0]].xy();
    c[1] = data_[y0 + xy_xy1[2]].xy();

    int32_t const y1 = width * xy_xy1[3];

    c[2] = data_[y1 + xy_xy1[0]].xy();
    c[3] = data_[y1 + xy_xy1[2]].xy();
}

void Float3::gather_3(int4_p xy_xy1, float3 c[4]) const {
    int32_t const width = dimensions_[0];

    int32_t const y0 = width * xy_xy1[1];

    c[0] = float3(data_[y0 + xy_xy1[0]]);
    c[1] = float3(data_[y0 + xy_xy1[2]]);

    int32_t const y1 = width * xy_xy1[3];

    c[2] = float3(data_[y1 + xy_xy1[0]]);
    c[3] = float3(data_[y1 + xy_xy1[2]]);
}

float Float3::at_element_1(int32_t x, int32_t y, int32_t element) const {
    int32_t const i     = (element * dimensions_[1] + y) * dimensions_[0] + x;
    auto const    value = data_[i];
    return value[0];
}

float2 Float3::at_element_2(int32_t x, int32_t y, int32_t element) const {
    int32_t const i     = (element * dimensions_[1] + y) * dimensions_[0] + x;
    auto const    value = data_[i];
    return value.xy();
}

float3 Float3::at_element_3(int32_t x, int32_t y, int32_t element) const {
    int32_t const i     = (element * dimensions_[1] + y) * dimensions_[0] + x;
    auto const    value = data_[i];
    return float3(value);
}

float Float3::at_1(int32_t x, int32_t y, int32_t z) const {
    int64_t const i = (int64_t(z) * int64_t(dimensions_[1]) + int64_t(y)) *
                          int64_t(dimensions_[0]) +
                      int64_t(x);
    auto const value = data_[i];
    return value[0];
}

float2 Float3::at_2(int32_t x, int32_t y, int32_t z) const {
    int64_t const i = (int64_t(z) * int64_t(dimensions_[1]) + int64_t(y)) *
                          int64_t(dimensions_[0]) +
                      int64_t(x);
    auto const value = data_[i];
    return value.xy();
}

float3 Float3::at_3(int32_t x, int32_t y, int32_t z) const {
    int64_t const i = (int64_t(z) * int64_t(dimensions_[1]) + int64_t(y)) *
                          int64_t(dimensions_[0]) +
                      int64_t(x);
    auto const value = data_[i];
    return float3(value);
}

float4 Float3::at_4(int32_t x, int32_t y, int32_t z) const {
    int64_t const i = (int64_t(z) * int64_t(dimensions_[1]) + int64_t(y)) *
                          int64_t(dimensions_[0]) +
                      int64_t(x);
    auto const value = data_[i];
    return float4(value, 1.f);
}

void Float3::gather_1(int3_p xyz, int3_p xyz1, float c[8]) const {
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
    c[0]             = data_[c0][0];

    int64_t const c1 = (d + y) * w + x1;
    c[1]             = data_[c1][0];

    int64_t const c2 = (d + y1) * w + x;
    c[2]             = data_[c2][0];

    int64_t const c3 = (d + y1) * w + x1;
    c[3]             = data_[c3][0];

    int64_t const d1 = z1 * h;

    int64_t const c4 = (d1 + y) * w + x;
    c[4]             = data_[c4][0];

    int64_t const c5 = (d1 + y) * w + x1;
    c[5]             = data_[c5][0];

    int64_t const c6 = (d1 + y1) * w + x;
    c[6]             = data_[c6][0];

    int64_t const c7 = (d1 + y1) * w + x1;
    c[7]             = data_[c7][0];
}

void Float3::gather_2(int3_p xyz, int3_p xyz1, float2 c[8]) const {
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
    c[0]             = data_[c0].xy();

    int64_t const c1 = (d + y) * w + x1;
    c[1]             = data_[c1].xy();

    int64_t const c2 = (d + y1) * w + x;
    c[2]             = data_[c2].xy();

    int64_t const c3 = (d + y1) * w + x1;
    c[3]             = data_[c3].xy();

    int64_t const d1 = z1 * h;

    int64_t const c4 = (d1 + y) * w + x;
    c[4]             = data_[c4].xy();

    int64_t const c5 = (d1 + y) * w + x1;
    c[5]             = data_[c5].xy();

    int64_t const c6 = (d1 + y1) * w + x;
    c[6]             = data_[c6].xy();

    int64_t const c7 = (d1 + y1) * w + x1;
    c[7]             = data_[c7].xy();
}

}  // namespace image::texture
