#include "texture_half_3.hpp"
#include "base/math/half.inl"
#include "base/math/vector4.inl"
#include "image/typed_image.hpp"

namespace image::texture {

Half3::Half3(image::Short3 const& image) noexcept : image_(image) {}

image::Short3 const& Half3::image() const noexcept {
    return image_;
}

int32_t Half3::num_elements() const noexcept {
    return image_.description().num_elements;
}

int2 Half3::dimensions_2() const noexcept {
    return image_.description().dimensions.xy();
}

int3 const& Half3::dimensions_3() const noexcept {
    return image_.description().dimensions;
}

float Half3::at_1(int32_t i) const noexcept {
    return half_to_float(image_.load(i)[0]);
}

float3 Half3::at_3(int32_t i) const noexcept {
    return half_to_float(image_.load(i));
}

float Half3::at_1(int32_t x, int32_t y) const noexcept {
    return half_to_float(image_.load(x, y)[0]);
}

float2 Half3::at_2(int32_t x, int32_t y) const noexcept {
    return half_to_float(image_.load(x, y).xy());
}

float3 Half3::at_3(int32_t x, int32_t y) const noexcept {
    return half_to_float(image_.load(x, y));
}

float4 Half3::at_4(int32_t x, int32_t y) const noexcept {
    return float4(half_to_float(image_.load(x, y)), 1.f);
}

void Half3::gather_1(int4 const& xy_xy1, float c[4]) const noexcept {
    short3 v[4];
    image_.gather(xy_xy1, v);

    c[0] = half_to_float(v[0][0]);
    c[1] = half_to_float(v[1][0]);
    c[2] = half_to_float(v[2][0]);
    c[3] = half_to_float(v[3][0]);
}

void Half3::gather_2(int4 const& xy_xy1, float2 c[4]) const noexcept {
    short3 v[4];
    image_.gather(xy_xy1, v);

    c[0] = half_to_float(v[0].xy());
    c[1] = half_to_float(v[1].xy());
    c[2] = half_to_float(v[2].xy());
    c[3] = half_to_float(v[3].xy());
}

void Half3::gather_3(int4 const& xy_xy1, float3 c[4]) const noexcept {
    short3 v[4];
    image_.gather(xy_xy1, v);

    c[0] = half_to_float(v[0]);
    c[1] = half_to_float(v[1]);
    c[2] = half_to_float(v[2]);
    c[3] = half_to_float(v[3]);
}

float Half3::at_element_1(int32_t x, int32_t y, int32_t element) const noexcept {
    return half_to_float(image_.at_element(x, y, element)[0]);
}

float2 Half3::at_element_2(int32_t x, int32_t y, int32_t element) const noexcept {
    return half_to_float(image_.at_element(x, y, element).xy());
}

float3 Half3::at_element_3(int32_t x, int32_t y, int32_t element) const noexcept {
    return half_to_float(image_.at_element(x, y, element));
}

float Half3::at_1(int32_t x, int32_t y, int32_t z) const noexcept {
    return half_to_float(image_.load(x, y, z)[0]);
}

float2 Half3::at_2(int32_t x, int32_t y, int32_t z) const noexcept {
    return half_to_float(image_.load(x, y, z).xy());
}

float3 Half3::at_3(int32_t x, int32_t y, int32_t z) const noexcept {
    return float3(image_.load(x, y, z));

    //	return float3(image_.at(x, y, z));
}

float4 Half3::at_4(int32_t x, int32_t y, int32_t z) const noexcept {
    return float4(half_to_float(image_.load(x, y, z)), 1.f);

    //	return float4(image_.at(x, y, z), 1.f);
}

}  // namespace image::texture
