#include "texture_float3.hpp"
#include "base/math/vector4.inl"
#include "image/typed_image.hpp"

namespace image::texture {

Float3::Float3(image::Float3 const& image) : image_(image) {}

image::Float3 const& Float3::image() const {
    return image_;
}

float Float3::at_1(int32_t i) const {
    return image_.load(i)[0];
}

float3 Float3::at_3(int32_t i) const {
    return float3(image_.load(i));
}

float Float3::at_1(int32_t x, int32_t y) const {
    return image_.load(x, y)[0];
}

float2 Float3::at_2(int32_t x, int32_t y) const {
    return image_.load(x, y).xy();
}

float3 Float3::at_3(int32_t x, int32_t y) const {
    return float3(image_.load(x, y));

    //	return float3(image_.at(x, y));
}

float4 Float3::at_4(int32_t x, int32_t y) const {
    return float4(image_.load(x, y), 1.f);
}

void Float3::gather_1(int4 const& xy_xy1, float c[4]) const {
    packed_float3 v[4];
    image_.gather(xy_xy1, v);

    c[0] = v[0][0];
    c[1] = v[1][0];
    c[2] = v[2][0];
    c[3] = v[3][0];
}

void Float3::gather_2(int4 const& xy_xy1, float2 c[4]) const {
    packed_float3 v[4];
    image_.gather(xy_xy1, v);

    c[0] = v[0].xy();
    c[1] = v[1].xy();
    c[2] = v[2].xy();
    c[3] = v[3].xy();
}

void Float3::gather_3(int4 const& xy_xy1, float3 c[4]) const {
    packed_float3 v[4];
    image_.gather(xy_xy1, v);

    c[0] = float3(v[0]);
    c[1] = float3(v[1]);
    c[2] = float3(v[2]);
    c[3] = float3(v[3]);
}

float Float3::at_element_1(int32_t x, int32_t y, int32_t element) const {
    return image_.at_element(x, y, element)[0];
}

float2 Float3::at_element_2(int32_t x, int32_t y, int32_t element) const {
    return image_.at_element(x, y, element).xy();
}

float3 Float3::at_element_3(int32_t x, int32_t y, int32_t element) const {
    //	return image_.at(x, y, element);

    return float3(image_.at_element(x, y, element));
}

float Float3::at_1(int32_t x, int32_t y, int32_t z) const {
    return image_.load(x, y, z)[0];
}

float2 Float3::at_2(int32_t x, int32_t y, int32_t z) const {
    return image_.load(x, y, z).xy();
}

float3 Float3::at_3(int32_t x, int32_t y, int32_t z) const {
    return float3(image_.load(x, y, z));

    //	return float3(image_.at(x, y, z));
}

float4 Float3::at_4(int32_t x, int32_t y, int32_t z) const {
    return float4(image_.load(x, y, z), 1.f);

    //	return float4(image_.at(x, y, z), 1.f);
}

}  // namespace image::texture
