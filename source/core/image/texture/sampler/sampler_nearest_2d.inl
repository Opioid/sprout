#pragma once

#include <algorithm>
#include "address_mode.hpp"
#include "image/texture/texture.hpp"
#include "sampler_nearest_2d.hpp"

namespace image::texture::sampler {

template <typename Address_mode_U, typename Address_mode_V>
float Nearest_2D<Address_mode_U, Address_mode_V>::sample_1(Texture const& texture, float2 uv) const
    noexcept {
    int2 const xy = map(texture, uv);

    return texture.at_1(xy[0], xy[1]);
}

template <typename Address_mode_U, typename Address_mode_V>
float2 Nearest_2D<Address_mode_U, Address_mode_V>::sample_2(Texture const& texture, float2 uv) const
    noexcept {
    int2 const xy = map(texture, uv);

    return texture.at_2(xy[0], xy[1]);
}

template <typename Address_mode_U, typename Address_mode_V>
float3 Nearest_2D<Address_mode_U, Address_mode_V>::sample_3(Texture const& texture, float2 uv) const
    noexcept {
    int2 const xy = map(texture, uv);

    return texture.at_3(xy[0], xy[1]);
}

template <typename Address_mode_U, typename Address_mode_V>
float Nearest_2D<Address_mode_U, Address_mode_V>::sample_1(Texture const& texture, float2 uv,
                                                           int32_t element) const noexcept {
    int2 const xy = map(texture, uv);

    int32_t const min_element = std::min(texture.num_elements() - 1, element);

    return texture.at_element_1(xy[0], xy[1], min_element);
}

template <typename Address_mode_U, typename Address_mode_V>
float2 Nearest_2D<Address_mode_U, Address_mode_V>::sample_2(Texture const& texture, float2 uv,
                                                            int32_t element) const noexcept {
    int2 const xy = map(texture, uv);

    int32_t const min_element = std::min(texture.num_elements() - 1, element);

    return texture.at_element_2(xy[0], xy[1], min_element);
}

template <typename Address_mode_U, typename Address_mode_V>
float3 Nearest_2D<Address_mode_U, Address_mode_V>::sample_3(Texture const& texture, float2 uv,
                                                            int32_t element) const noexcept {
    int2 const xy = map(texture, uv);

    int32_t const min_element = std::min(texture.num_elements() - 1, element);

    return texture.at_element_3(xy[0], xy[1], min_element);
}

template <typename Address_mode_U, typename Address_mode_V>
float2 Nearest_2D<Address_mode_U, Address_mode_V>::address(float2 uv) const noexcept {
    return float2(Address_mode_U::f(uv[0]), Address_mode_V::f(uv[1]));
}

template <typename Address_mode_U, typename Address_mode_V>
int2 Nearest_2D<Address_mode_U, Address_mode_V>::map(Texture const& texture, float2 uv) noexcept {
    auto const b = texture.back_2();
    auto const d = texture.dimensions_float2();

    float const u = Address_mode_U::f(uv[0]);
    float const v = Address_mode_V::f(uv[1]);

    return int2(std::min(static_cast<int32_t>(u * d[0]), b[0]),
                std::min(static_cast<int32_t>(v * d[1]), b[1]));
}

}  // namespace image::texture::sampler
