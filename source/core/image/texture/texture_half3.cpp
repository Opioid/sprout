#include "texture_half3.hpp"
#include "base/math/half.inl"
#include "base/math/vector4.inl"
#include "image/typed_image.hpp"

namespace image::texture {

Half3::Half3(image::Short3 const& image) : image_(image) {}

image::Short3 const& Half3::image() const {
    return image_;
}

float Half3::at_1(int32_t x, int32_t y) const {
    return half_to_float(image_.at(x, y)[0]);
}

float2 Half3::at_2(int32_t x, int32_t y) const {
    return half_to_float(image_.at(x, y).xy());
}

float3 Half3::at_3(int32_t x, int32_t y) const {
    return half_to_float(image_.at(x, y));
}

float4 Half3::at_4(int32_t x, int32_t y) const {
    return float4(half_to_float(image_.at(x, y)), 1.f);
}

void Half3::gather_1(int4 const& xy_xy1, float c[4]) const {
    ushort3 v[4];
    image_.gather(xy_xy1, v);

    c[0] = half_to_float(v[0][0]);
    c[1] = half_to_float(v[1][0]);
    c[2] = half_to_float(v[2][0]);
    c[3] = half_to_float(v[3][0]);
}

void Half3::gather_2(int4 const& xy_xy1, float2 c[4]) const {
    ushort3 v[4];
    image_.gather(xy_xy1, v);

    c[0] = half_to_float(v[0].xy());
    c[1] = half_to_float(v[1].xy());
    c[2] = half_to_float(v[2].xy());
    c[3] = half_to_float(v[3].xy());
}

void Half3::gather_3(int4 const& xy_xy1, float3 c[4]) const {
    ushort3 v[4];
    image_.gather(xy_xy1, v);

    c[0] = half_to_float(v[0]);
    c[1] = half_to_float(v[1]);
    c[2] = half_to_float(v[2]);
    c[3] = half_to_float(v[3]);
}

float Half3::at_element_1(int32_t x, int32_t y, int32_t element) const {
    return half_to_float(image_.at_element(x, y, element)[0]);
}

float2 Half3::at_element_2(int32_t x, int32_t y, int32_t element) const {
    return half_to_float(image_.at_element(x, y, element).xy());
}

float3 Half3::at_element_3(int32_t x, int32_t y, int32_t element) const {
    return half_to_float(image_.at_element(x, y, element));
}

float Half3::at_1(int32_t x, int32_t y, int32_t z) const {
    return half_to_float(image_.at(x, y, z)[0]);
}

float2 Half3::at_2(int32_t x, int32_t y, int32_t z) const {
    return half_to_float(image_.at(x, y, z).xy());
}

float3 Half3::at_3(int32_t x, int32_t y, int32_t z) const {
    return half_to_float(image_.at(x, y, z));
}

float4 Half3::at_4(int32_t x, int32_t y, int32_t z) const {
    return float4(half_to_float(image_.at(x, y, z)), 1.f);

    //	return float4(image_.at(x, y, z), 1.f);
}

void Half3::gather_1(int3 const& xyz, int3 const& xyz1, float c[8]) const {}

void Half3::gather_2(int3 const& xyz, int3 const& xyz1, float2 c[8]) const {}

}  // namespace image::texture
