#include "texture_half4.hpp"
#include "base/math/half.inl"
#include "base/math/vector4.inl"
#include "image/typed_image.hpp"

namespace image::texture {

Half4::Half4(image::Short4 const& image) : image_(image) {}

image::Short4 const& Half4::image() const {
    return image_;
}

int32_t Half4::num_elements() const {
    return image_.description().num_elements_;
}

int2 Half4::dimensions_2() const {
    return image_.description().dimensions_.xy();
}

int3 const& Half4::dimensions_3() const {
    return image_.description().dimensions_;
}

float Half4::at_1(int32_t i) const {
    return half_to_float(image_.load(i)[0]);
}

float3 Half4::at_3(int32_t i) const {
    return half_to_float(image_.load(i).xyz());
}

float Half4::at_1(int32_t x, int32_t y) const {
    return half_to_float(image_.load(x, y)[0]);
}

float2 Half4::at_2(int32_t x, int32_t y) const {
    return half_to_float(image_.load(x, y).xy());
}

float3 Half4::at_3(int32_t x, int32_t y) const {
    return half_to_float(image_.load(x, y).xyz());
}

float4 Half4::at_4(int32_t x, int32_t y) const {
    return half_to_float(image_.load(x, y));
}

void Half4::gather_1(int4 const& xy_xy1, float c[4]) const {
    short4 v[4];
    image_.gather(xy_xy1, v);

    c[0] = half_to_float(v[0][0]);
    c[1] = half_to_float(v[1][0]);
    c[2] = half_to_float(v[2][0]);
    c[3] = half_to_float(v[3][0]);
}

void Half4::gather_2(int4 const& xy_xy1, float2 c[4]) const {
    short4 v[4];
    image_.gather(xy_xy1, v);

    c[0] = half_to_float(v[0].xy());
    c[1] = half_to_float(v[1].xy());
    c[2] = half_to_float(v[2].xy());
    c[3] = half_to_float(v[3].xy());
}

void Half4::gather_3(int4 const& xy_xy1, float3 c[4]) const {
    short4 v[4];
    image_.gather(xy_xy1, v);

    c[0] = half_to_float(v[0].xyz());
    c[1] = half_to_float(v[1].xyz());
    c[2] = half_to_float(v[2].xyz());
    c[3] = half_to_float(v[3].xyz());
}

float Half4::at_element_1(int32_t x, int32_t y, int32_t element) const {
    return half_to_float(image_.at_element(x, y, element)[0]);
}

float2 Half4::at_element_2(int32_t x, int32_t y, int32_t element) const {
    return half_to_float(image_.at_element(x, y, element).xy());
}

float3 Half4::at_element_3(int32_t x, int32_t y, int32_t element) const {
    return half_to_float(image_.at_element(x, y, element).xyz());
}

float Half4::at_1(int32_t x, int32_t y, int32_t z) const {
    return half_to_float(image_.load(x, y, z)[0]);
}

float2 Half4::at_2(int32_t x, int32_t y, int32_t z) const {
    return half_to_float(image_.load(x, y, z).xy());
}

float3 Half4::at_3(int32_t x, int32_t y, int32_t z) const {
    return half_to_float(image_.load(x, y, z).xyz());
}

float4 Half4::at_4(int32_t x, int32_t y, int32_t z) const {
    return half_to_float(image_.load(x, y, z));
}

}  // namespace image::texture
