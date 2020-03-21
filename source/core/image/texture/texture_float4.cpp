#include "texture_float4.hpp"
#include "base/math/vector4.inl"
#include "image/typed_image.hpp"

namespace image::texture {

Float4::Float4(image::Float4 const& image) : image_(image) {}

image::Float4 const& Float4::image() const {
    return image_;
}

float Float4::at_1(int32_t i) const {
    return image_.load(i)[0];
}

float3 Float4::at_3(int32_t i) const {
    return image_.load(i).xyz();
}

float Float4::at_1(int32_t x, int32_t y) const {
    return image_.load(x, y)[0];
}

float2 Float4::at_2(int32_t x, int32_t y) const {
    return image_.load(x, y).xy();
}

float3 Float4::at_3(int32_t x, int32_t y) const {
    return image_.load(x, y).xyz();

    //	return float3(image_.at(x, y));
}

float4 Float4::at_4(int32_t x, int32_t y) const {
    return image_.load(x, y);
}

void Float4::gather_1(int4 const& xy_xy1, float c[4]) const {
    float4 v[4];
    image_.gather(xy_xy1, v);

    c[0] = v[0][0];
    c[1] = v[1][0];
    c[2] = v[2][0];
    c[3] = v[3][0];
}

void Float4::gather_2(int4 const& xy_xy1, float2 c[4]) const {
    float4 v[4];
    image_.gather(xy_xy1, v);

    c[0] = v[0].xy();
    c[1] = v[1].xy();
    c[2] = v[2].xy();
    c[3] = v[3].xy();
}

void Float4::gather_3(int4 const& xy_xy1, float3 c[4]) const {
    float4 v[4];
    image_.gather(xy_xy1, v);

    c[0] = v[0].xyz();
    c[1] = v[1].xyz();
    c[2] = v[2].xyz();
    c[3] = v[3].xyz();
}

float Float4::at_element_1(int32_t x, int32_t y, int32_t element) const {
    return image_.at_element(x, y, element)[0];
}

float2 Float4::at_element_2(int32_t x, int32_t y, int32_t element) const {
    return image_.at_element(x, y, element).xy();
}

float3 Float4::at_element_3(int32_t x, int32_t y, int32_t element) const {
    return image_.at_element(x, y, element).xyz();
}

float Float4::at_1(int32_t x, int32_t y, int32_t z) const {
    return image_.load(x, y, z)[0];
}

float2 Float4::at_2(int32_t x, int32_t y, int32_t z) const {
    return image_.load(x, y, z).xy();
}

float3 Float4::at_3(int32_t x, int32_t y, int32_t z) const {
    return image_.load(x, y, z).xyz();
}

float4 Float4::at_4(int32_t x, int32_t y, int32_t z) const {
    return image_.load(x, y, z);
}

}  // namespace image::texture
