#include "texture_float2.hpp"
#include "base/math/vector4.inl"
#include "image/typed_image.hpp"

namespace image::texture {

Float2::Float2(image::Float2 const& image) : image_(image) {}

image::Float2 const& Float2::image() const {
    return image_;
}

float Float2::at_1(int32_t x, int32_t y) const {
    return image_.at(x, y)[0];
}

float2 Float2::at_2(int32_t x, int32_t y) const {
    return image_.at(x, y);
}

float3 Float2::at_3(int32_t x, int32_t y) const {
    return float3(image_.at(x, y), 0.f);
}

float4 Float2::at_4(int32_t x, int32_t y) const {
    return float4(image_.at(x, y), 0.f, 1.f);
}

void Float2::gather_1(int4 const& xy_xy1, float c[4]) const {
    float2 v[4];
    image_.gather(xy_xy1, v);

    c[0] = v[0][0];
    c[1] = v[1][0];
    c[2] = v[2][0];
    c[3] = v[3][0];
}

void Float2::gather_2(int4 const& xy_xy1, float2 c[4]) const {
    image_.gather(xy_xy1, c);
}

void Float2::gather_3(int4 const& xy_xy1, float3 c[4]) const {
    float2 v[4];
    image_.gather(xy_xy1, v);

    c[0] = float3(v[0], 0.f);
    c[1] = float3(v[1], 0.f);
    c[2] = float3(v[2], 0.f);
    c[3] = float3(v[3], 0.f);
}

float Float2::at_element_1(int32_t x, int32_t y, int32_t element) const {
    return image_.at_element(x, y, element)[0];
}

float2 Float2::at_element_2(int32_t x, int32_t y, int32_t element) const {
    return image_.at_element(x, y, element);
}

float3 Float2::at_element_3(int32_t x, int32_t y, int32_t element) const {
    return float3(image_.at_element(x, y, element), 0.f);
}

float Float2::at_1(int32_t x, int32_t y, int32_t z) const {
    return image_.at(x, y, z)[0];
}

float2 Float2::at_2(int32_t x, int32_t y, int32_t z) const {
    return image_.at(x, y, z);
}

float3 Float2::at_3(int32_t x, int32_t y, int32_t z) const {
    return float3(image_.at(x, y, z), 0.f);
}

float4 Float2::at_4(int32_t x, int32_t y, int32_t z) const {
    return float4(image_.at(x, y, z), 0.f, 1.f);
}

void Float2::gather_1(int3 const& xyz, int3 const& xyz1, float c[8]) const {
    float2 v[8];

    image_.gather(xyz, xyz1, v);

    c[0] = v[0][0];
    c[1] = v[1][0];
    c[2] = v[2][0];
    c[3] = v[3][0];
    c[4] = v[4][0];
    c[5] = v[5][0];
    c[6] = v[6][0];
    c[7] = v[7][0];
}

void Float2::gather_2(int3 const& xyz, int3 const& xyz1, float2 c[8]) const {
    image_.gather(xyz, xyz1, c);
}

}  // namespace image::texture
