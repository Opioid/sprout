#include "texture_float1.hpp"
#include "base/math/vector4.inl"
#include "image/typed_image.hpp"

namespace image::texture {

Float1::Float1(image::Float1 const& image)
    : description_(image.description()),
      data_(image.data()),
      dimensions_(image.description().dimensions()) {}

Description const& Float1::description() const {
    return description_;
}

float Float1::at_1(int32_t x, int32_t y) const {
    int32_t const i     = y * dimensions_[0] + x;
    auto const    value = data_[i];
    return value;
}

float2 Float1::at_2(int32_t x, int32_t y) const {
    int32_t const i     = y * dimensions_[0] + x;
    auto const    value = data_[i];
    return float2(value, 0.f);
}

float3 Float1::at_3(int32_t x, int32_t y) const {
    int32_t const i     = y * dimensions_[0] + x;
    auto const    value = data_[i];
    return float3(value, 0.f, 0.f);
}

float4 Float1::at_4(int32_t x, int32_t y) const {
    int32_t const i     = y * dimensions_[0] + x;
    auto const    value = data_[i];
    return float4(value, 0.f, 0.f, 1.f);
}

void Float1::gather_1(int4_p xy_xy1, float c[4]) const {
    int32_t const width = dimensions_[0];

    int32_t const y0 = width * xy_xy1[1];

    c[0] = data_[y0 + xy_xy1[0]];
    c[1] = data_[y0 + xy_xy1[2]];

    int32_t const y1 = width * xy_xy1[3];

    c[2] = data_[y1 + xy_xy1[0]];
    c[3] = data_[y1 + xy_xy1[2]];
}

void Float1::gather_2(int4_p xy_xy1, float2 c[4]) const {
    int32_t const width = dimensions_[0];

    int32_t const y0 = width * xy_xy1[1];

    c[0] = float2(data_[y0 + xy_xy1[0]], 0.f);
    c[1] = float2(data_[y0 + xy_xy1[2]], 0.f);

    int32_t const y1 = width * xy_xy1[3];

    c[2] = float2(data_[y1 + xy_xy1[0]], 0.f);
    c[3] = float2(data_[y1 + xy_xy1[2]], 0.f);
}

void Float1::gather_3(int4_p xy_xy1, float3 c[4]) const {
    int32_t const width = dimensions_[0];

    int32_t const y0 = width * xy_xy1[1];

    c[0] = float3(data_[y0 + xy_xy1[0]], 0.f, 0.f);
    c[1] = float3(data_[y0 + xy_xy1[2]], 0.f, 0.f);

    int32_t const y1 = width * xy_xy1[3];

    c[2] = float3(data_[y1 + xy_xy1[0]], 0.f, 0.f);
    c[3] = float3(data_[y1 + xy_xy1[2]], 0.f, 0.f);
}

float Float1::at_element_1(int32_t x, int32_t y, int32_t element) const {
    int32_t const i     = (element * dimensions_[1] + y) * dimensions_[0] + x;
    auto const    value = data_[i];
    return value;
}

float2 Float1::at_element_2(int32_t x, int32_t y, int32_t element) const {
    int32_t const i     = (element * dimensions_[1] + y) * dimensions_[0] + x;
    auto const    value = data_[i];
    return float2(value, 0.f);
}

float3 Float1::at_element_3(int32_t x, int32_t y, int32_t element) const {
    int32_t const i     = (element * dimensions_[1] + y) * dimensions_[0] + x;
    auto const    value = data_[i];
    return float3(value, 0.f, 0.f);
}

float Float1::at_1(int32_t x, int32_t y, int32_t z) const {
    int64_t const i = (int64_t(z) * int64_t(dimensions_[1]) + int64_t(y)) *
                          int64_t(dimensions_[0]) +
                      int64_t(x);
    auto const value = data_[i];
    return value;
}

float2 Float1::at_2(int32_t x, int32_t y, int32_t z) const {
    int64_t const i = (int64_t(z) * int64_t(dimensions_[1]) + int64_t(y)) *
                          int64_t(dimensions_[0]) +
                      int64_t(x);
    auto const value = data_[i];
    return float2(value, 0.f);
}

float3 Float1::at_3(int32_t x, int32_t y, int32_t z) const {
    int64_t const i = (int64_t(z) * int64_t(dimensions_[1]) + int64_t(y)) *
                          int64_t(dimensions_[0]) +
                      int64_t(x);
    auto const value = data_[i];
    return float3(value, 0.f, 0.f);
}

float4 Float1::at_4(int32_t x, int32_t y, int32_t z) const {
    int64_t const i = (int64_t(z) * int64_t(dimensions_[1]) + int64_t(y)) *
                          int64_t(dimensions_[0]) +
                      int64_t(x);
    auto const value = data_[i];
    return float4(value, 0.f, 0.f, 1.f);
}

void Float1::gather_1(int3_p xyz, int3_p xyz1, float c[8]) const {
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
    c[0]             = data_[c0];

    int64_t const c1 = (d + y) * w + x1;
    c[1]             = data_[c1];

    int64_t const c2 = (d + y1) * w + x;
    c[2]             = data_[c2];

    int64_t const c3 = (d + y1) * w + x1;
    c[3]             = data_[c3];

    int64_t const d1 = z1 * h;

    int64_t const c4 = (d1 + y) * w + x;
    c[4]             = data_[c4];

    int64_t const c5 = (d1 + y) * w + x1;
    c[5]             = data_[c5];

    int64_t const c6 = (d1 + y1) * w + x;
    c[6]             = data_[c6];

    int64_t const c7 = (d1 + y1) * w + x1;
    c[7]             = data_[c7];
}

void Float1::gather_2(int3_p xyz, int3_p xyz1, float2 c[8]) const {
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
    c[0]             = float2(data_[c0], 0.f);

    int64_t const c1 = (d + y) * w + x1;
    c[1]             = float2(data_[c1], 0.f);

    int64_t const c2 = (d + y1) * w + x;
    c[2]             = float2(data_[c2], 0.f);

    int64_t const c3 = (d + y1) * w + x1;
    c[3]             = float2(data_[c3], 0.f);

    int64_t const d1 = z1 * h;

    int64_t const c4 = (d1 + y) * w + x;
    c[4]             = float2(data_[c4], 0.f);

    int64_t const c5 = (d1 + y) * w + x1;
    c[5]             = float2(data_[c5], 0.f);

    int64_t const c6 = (d1 + y1) * w + x;
    c[6]             = float2(data_[c6], 0.f);

    int64_t const c7 = (d1 + y1) * w + x1;
    c[7]             = float2(data_[c7], 0.f);
}

Float1_sparse::Float1_sparse(image::Float1_sparse const& image)
    : description_(image.description()),
      image_(image),
      dimensions_(image.description().dimensions()) {}

Description const& Float1_sparse::description() const {
    return description_;
}

float Float1_sparse::at_1(int32_t x, int32_t y) const {
    return image_.at(x, y);
}

float2 Float1_sparse::at_2(int32_t x, int32_t y) const {
    return float2(image_.at(x, y), 0.f);
}

float3 Float1_sparse::at_3(int32_t x, int32_t y) const {
    return float3(image_.at(x, y), 0.f, 0.f);
}

float4 Float1_sparse::at_4(int32_t x, int32_t y) const {
    return float4(image_.at(x, y), 0.f, 0.f, 1.f);
}

void Float1_sparse::gather_1(int4_p xy_xy1, float c[4]) const {
    image_.gather(xy_xy1, c);
}

void Float1_sparse::gather_2(int4_p xy_xy1, float2 c[4]) const {
    float v[4];
    image_.gather(xy_xy1, v);

    c[0] = float2(v[0], 0.f);
    c[1] = float2(v[1], 0.f);
    c[2] = float2(v[2], 0.f);
    c[3] = float2(v[3], 0.f);
}

void Float1_sparse::gather_3(int4_p xy_xy1, float3 c[4]) const {
    float v[4];
    image_.gather(xy_xy1, v);

    c[0] = float3(v[0], 0.f, 0.f);
    c[1] = float3(v[1], 0.f, 0.f);
    c[2] = float3(v[2], 0.f, 0.f);
    c[3] = float3(v[3], 0.f, 0.f);
}

float Float1_sparse::at_element_1(int32_t x, int32_t y, int32_t element) const {
    return image_.at_element(x, y, element);
}

float2 Float1_sparse::at_element_2(int32_t x, int32_t y, int32_t element) const {
    return float2(image_.at_element(x, y, element), 0.f);
}

float3 Float1_sparse::at_element_3(int32_t x, int32_t y, int32_t element) const {
    return float3(image_.at_element(x, y, element), 0.f, 0.f);
}

float Float1_sparse::at_1(int32_t x, int32_t y, int32_t z) const {
    return image_.at(x, y, z);
}

float2 Float1_sparse::at_2(int32_t x, int32_t y, int32_t z) const {
    return float2(image_.at(x, y, z), 0.f);
}

float3 Float1_sparse::at_3(int32_t x, int32_t y, int32_t z) const {
    return float3(image_.at(x, y, z), 0.f, 0.f);
}

float4 Float1_sparse::at_4(int32_t x, int32_t y, int32_t z) const {
    return float4(image_.at(x, y, z), 0.f, 0.f, 1.f);
}

void Float1_sparse::gather_1(int3_p xyz, int3_p xyz1, float c[8]) const {
    image_.gather(xyz, xyz1, c);
}

void Float1_sparse::gather_2(int3_p xyz, int3_p xyz1, float2 c[8]) const {
    float v[8];
    image_.gather(xyz, xyz1, v);

    c[0] = float2(v[0], 0.f);
    c[1] = float2(v[1], 0.f);
    c[2] = float2(v[2], 0.f);
    c[3] = float2(v[3], 0.f);
    c[4] = float2(v[4], 0.f);
    c[5] = float2(v[5], 0.f);
    c[6] = float2(v[6], 0.f);
    c[7] = float2(v[7], 0.f);
}

}  // namespace image::texture
