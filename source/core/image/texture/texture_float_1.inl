#ifndef SU_CORE_IMAGE_TEXTURE_FLOAT1_INL
#define SU_CORE_IMAGE_TEXTURE_FLOAT1_INL

#include "base/math/vector4.inl"
#include "image/typed_image.inl"
#include "texture_float_1.hpp"

namespace image::texture {

template <typename T>
Float1_t<T>::Float1_t(std::shared_ptr<Image> const& image) noexcept
    : Texture(image), image_(*reinterpret_cast<T const*>(image.get())) {}

template <typename T>
float Float1_t<T>::at_1(int32_t i) const noexcept {
    return image_.load(i);
}

template <typename T>
float3 Float1_t<T>::at_3(int32_t i) const noexcept {
    return float3(image_.load(i), 0.f, 0.f);
}

template <typename T>
float Float1_t<T>::at_1(int32_t x, int32_t y) const noexcept {
    return image_.load(x, y);
}

template <typename T>
float2 Float1_t<T>::at_2(int32_t x, int32_t y) const noexcept {
    return float2(image_.load(x, y), 0.f);
}

template <typename T>
float3 Float1_t<T>::at_3(int32_t x, int32_t y) const noexcept {
    return float3(image_.load(x, y), 0.f, 0.f);
}

template <typename T>
void Float1_t<T>::gather_1(int4 const& xy_xy1, float c[4]) const noexcept {
    image_.gather(xy_xy1, c);
}

template <typename T>
void Float1_t<T>::gather_2(int4 const& xy_xy1, float2 c[4]) const noexcept {
    float v[4];
    image_.gather(xy_xy1, v);

    c[0] = float2(v[0], 0.f);
    c[1] = float2(v[1], 0.f);
    c[2] = float2(v[2], 0.f);
    c[3] = float2(v[3], 0.f);
}

template <typename T>
void Float1_t<T>::gather_3(int4 const& xy_xy1, float3 c[4]) const noexcept {
    float v[4];
    image_.gather(xy_xy1, v);

    c[0] = float3(v[0], 0.f, 0.f);
    c[1] = float3(v[1], 0.f, 0.f);
    c[2] = float3(v[2], 0.f, 0.f);
    c[3] = float3(v[3], 0.f, 0.f);
}

template <typename T>
float Float1_t<T>::at_element_1(int32_t x, int32_t y, int32_t element) const noexcept {
    return image_.at_element(x, y, element);
}

template <typename T>
float2 Float1_t<T>::at_element_2(int32_t x, int32_t y, int32_t element) const noexcept {
    return float2(image_.at_element(x, y, element), 0.f);
}

template <typename T>
float3 Float1_t<T>::at_element_3(int32_t x, int32_t y, int32_t element) const noexcept {
    return float3(image_.at_element(x, y, element), 0.f, 0.f);
}

template <typename T>
float Float1_t<T>::at_1(int32_t x, int32_t y, int32_t z) const noexcept {
    return image_.load(x, y, z);
}

template <typename T>
float2 Float1_t<T>::at_2(int32_t x, int32_t y, int32_t z) const noexcept {
    return float2(image_.load(x, y, z), 0.f);
}

template <typename T>
float3 Float1_t<T>::at_3(int32_t x, int32_t y, int32_t z) const noexcept {
    return float3(image_.load(x, y, z), 0.f, 0.f);
}

}  // namespace image::texture

#endif
