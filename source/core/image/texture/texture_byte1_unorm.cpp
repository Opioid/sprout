#include "texture_byte1_unorm.hpp"
#include "base/math/vector4.inl"
#include "image/typed_image.hpp"
#include "texture_encoding.inl"

namespace image::texture {

Byte1_unorm::Byte1_unorm(Byte1 const& image)
    : description_(image.description()),
      data_(image.data()),
      dimensions_(image.description().dimensions()) {}

Description const& Byte1_unorm::description() const {
    return description_;
}

float Byte1_unorm::at_1(int32_t x, int32_t y) const {
    int32_t const i     = y * dimensions_[0] + x;
    auto const    value = data_[i];
    return encoding::cached_unorm_to_float(value);
}

float2 Byte1_unorm::at_2(int32_t x, int32_t y) const {
    int32_t const i     = y * dimensions_[0] + x;
    auto const    value = data_[i];
    return float2(encoding::cached_unorm_to_float(value), 0.f);
}

float3 Byte1_unorm::at_3(int32_t x, int32_t y) const {
    int32_t const i     = y * dimensions_[0] + x;
    auto const    value = data_[i];
    return float3(encoding::cached_unorm_to_float(value), 0.f, 0.f);
}

float4 Byte1_unorm::at_4(int32_t x, int32_t y) const {
    int32_t const i     = y * dimensions_[0] + x;
    auto const    value = data_[i];
    return float4(encoding::cached_unorm_to_float(value), 0.f, 0.f, 1.f);
}

void Byte1_unorm::gather_1(int4_p xy_xy1, float c[4]) const {
    int32_t const width = dimensions_[0];

    int32_t const y0 = width * xy_xy1[1];

    uint8_t const v0 = data_[y0 + xy_xy1[0]];
    uint8_t const v1 = data_[y0 + xy_xy1[2]];

    int32_t const y1 = width * xy_xy1[3];

    uint8_t const v2 = data_[y1 + xy_xy1[0]];
    uint8_t const v3 = data_[y1 + xy_xy1[2]];

    c[0] = encoding::cached_unorm_to_float(v0);
    c[1] = encoding::cached_unorm_to_float(v1);
    c[2] = encoding::cached_unorm_to_float(v2);
    c[3] = encoding::cached_unorm_to_float(v3);
}

void Byte1_unorm::gather_2(int4_p xy_xy1, float2 c[4]) const {
    int32_t const width = dimensions_[0];

    int32_t const y0 = width * xy_xy1[1];

    uint8_t const v0 = data_[y0 + xy_xy1[0]];
    uint8_t const v1 = data_[y0 + xy_xy1[2]];

    int32_t const y1 = width * xy_xy1[3];

    uint8_t const v2 = data_[y1 + xy_xy1[0]];
    uint8_t const v3 = data_[y1 + xy_xy1[2]];

    c[0] = float2(encoding::cached_unorm_to_float(v0), 0.f);
    c[1] = float2(encoding::cached_unorm_to_float(v1), 0.f);
    c[2] = float2(encoding::cached_unorm_to_float(v2), 0.f);
    c[3] = float2(encoding::cached_unorm_to_float(v3), 0.f);
}

void Byte1_unorm::gather_3(int4_p xy_xy1, float3 c[4]) const {
    int32_t const width = dimensions_[0];

    int32_t const y0 = width * xy_xy1[1];

    uint8_t const v0 = data_[y0 + xy_xy1[0]];
    uint8_t const v1 = data_[y0 + xy_xy1[2]];

    int32_t const y1 = width * xy_xy1[3];

    uint8_t const v2 = data_[y1 + xy_xy1[0]];
    uint8_t const v3 = data_[y1 + xy_xy1[2]];

    c[0] = float3(encoding::cached_unorm_to_float(v0), 0.f, 0.f);
    c[1] = float3(encoding::cached_unorm_to_float(v1), 0.f, 0.f);
    c[2] = float3(encoding::cached_unorm_to_float(v2), 0.f, 0.f);
    c[3] = float3(encoding::cached_unorm_to_float(v3), 0.f, 0.f);
}

float Byte1_unorm::at_element_1(int32_t x, int32_t y, int32_t element) const {
    int32_t const i     = (element * dimensions_[1] + y) * dimensions_[0] + x;
    uint8_t       value = data_[i];
    return encoding::cached_unorm_to_float(value);
}

float2 Byte1_unorm::at_element_2(int32_t x, int32_t y, int32_t element) const {
    int32_t const i     = (element * dimensions_[1] + y) * dimensions_[0] + x;
    uint8_t       value = data_[i];
    return float2(encoding::cached_unorm_to_float(value), 0.f);
}

float3 Byte1_unorm::at_element_3(int32_t x, int32_t y, int32_t element) const {
    int32_t const i     = (element * dimensions_[1] + y) * dimensions_[0] + x;
    uint8_t       value = data_[i];
    return float3(encoding::cached_unorm_to_float(value), 0.f, 0.f);
}

float Byte1_unorm::at_1(int32_t x, int32_t y, int32_t z) const {
    int64_t const i = (int64_t(z) * int64_t(dimensions_[1]) + int64_t(y)) *
                          int64_t(dimensions_[0]) +
                      int64_t(x);
    uint8_t value = data_[i];
    return encoding::cached_unorm_to_float(value);
}

float2 Byte1_unorm::at_2(int32_t x, int32_t y, int32_t z) const {
    int64_t const i = (int64_t(z) * int64_t(dimensions_[1]) + int64_t(y)) *
                          int64_t(dimensions_[0]) +
                      int64_t(x);
    uint8_t value = data_[i];
    return float2(encoding::cached_unorm_to_float(value), 0.f);
}

float3 Byte1_unorm::at_3(int32_t x, int32_t y, int32_t z) const {
    int64_t const i = (int64_t(z) * int64_t(dimensions_[1]) + int64_t(y)) *
                          int64_t(dimensions_[0]) +
                      int64_t(x);
    uint8_t value = data_[i];
    return float3(encoding::cached_unorm_to_float(value), 0.f, 0.f);
}

float4 Byte1_unorm::at_4(int32_t x, int32_t y, int32_t z) const {
    int64_t const i = (int64_t(z) * int64_t(dimensions_[1]) + int64_t(y)) *
                          int64_t(dimensions_[0]) +
                      int64_t(x);
    uint8_t value = data_[i];
    return float4(encoding::cached_unorm_to_float(value), 0.f, 0.f, 1.f);
}

void Byte1_unorm::gather_1(int3_p xyz, int3_p xyz1, float c[8]) const {
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
    uint8_t const v0 = data_[c0];

    int64_t const c1 = (d + y) * w + x1;
    uint8_t const v1 = data_[c1];

    int64_t const c2 = (d + y1) * w + x;
    uint8_t const v2 = data_[c2];

    int64_t const c3 = (d + y1) * w + x1;
    uint8_t const v3 = data_[c3];

    int64_t const d1 = z1 * h;

    int64_t const c4 = (d1 + y) * w + x;
    uint8_t const v4 = data_[c4];

    int64_t const c5 = (d1 + y) * w + x1;
    uint8_t const v5 = data_[c5];

    int64_t const c6 = (d1 + y1) * w + x;
    uint8_t const v6 = data_[c6];

    int64_t const c7 = (d1 + y1) * w + x1;
    uint8_t const v7 = data_[c7];

    c[0] = encoding::cached_unorm_to_float(v0);
    c[1] = encoding::cached_unorm_to_float(v1);
    c[2] = encoding::cached_unorm_to_float(v2);
    c[3] = encoding::cached_unorm_to_float(v3);
    c[4] = encoding::cached_unorm_to_float(v4);
    c[5] = encoding::cached_unorm_to_float(v5);
    c[6] = encoding::cached_unorm_to_float(v6);
    c[7] = encoding::cached_unorm_to_float(v7);
}

void Byte1_unorm::gather_2(int3_p xyz, int3_p xyz1, float2 c[8]) const {
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
    uint8_t const v0 = data_[c0];

    int64_t const c1 = (d + y) * w + x1;
    uint8_t const v1 = data_[c1];

    int64_t const c2 = (d + y1) * w + x;
    uint8_t const v2 = data_[c2];

    int64_t const c3 = (d + y1) * w + x1;
    uint8_t const v3 = data_[c3];

    int64_t const d1 = z1 * h;

    int64_t const c4 = (d1 + y) * w + x;
    uint8_t const v4 = data_[c4];

    int64_t const c5 = (d1 + y) * w + x1;
    uint8_t const v5 = data_[c5];

    int64_t const c6 = (d1 + y1) * w + x;
    uint8_t const v6 = data_[c6];

    int64_t const c7 = (d1 + y1) * w + x1;
    uint8_t const v7 = data_[c7];

    c[0] = float2(encoding::cached_unorm_to_float(v0), 0.f);
    c[1] = float2(encoding::cached_unorm_to_float(v1), 0.f);
    c[2] = float2(encoding::cached_unorm_to_float(v2), 0.f);
    c[3] = float2(encoding::cached_unorm_to_float(v3), 0.f);
    c[4] = float2(encoding::cached_unorm_to_float(v4), 0.f);
    c[5] = float2(encoding::cached_unorm_to_float(v5), 0.f);
    c[6] = float2(encoding::cached_unorm_to_float(v6), 0.f);
    c[7] = float2(encoding::cached_unorm_to_float(v7), 0.f);
}

}  // namespace image::texture
