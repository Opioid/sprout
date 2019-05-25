#ifndef SU_CORE_IMAGE_TEXTURE_SAMPLER_LINEAR_2D_INL
#define SU_CORE_IMAGE_TEXTURE_SAMPLER_LINEAR_2D_INL

#include <algorithm>
#include "address_mode.hpp"
#include "bilinear.hpp"
#include "image/texture/texture.inl"
#include "sampler_linear_2d.hpp"

namespace image::texture::sampler {

template <typename Address_U, typename Address_V>
float Linear_2D<Address_U, Address_V>::sample_1(Texture const& texture, float2 uv) const noexcept {
    int4         xy_xy1;
    float2 const st = map(texture, uv, xy_xy1);

    float c[4];
    texture.gather_1(xy_xy1, c);

    return bilinear(c, st[0], st[1]);
}

template <typename Address_U, typename Address_V>
float2 Linear_2D<Address_U, Address_V>::sample_2(Texture const& texture, float2 uv) const noexcept {
    int4         xy_xy1;
    float2 const st = map(texture, uv, xy_xy1);

    float2 c[4];
    texture.gather_2(xy_xy1, c);

    return bilinear(c, st[0], st[1]);
}

template <typename Address_U, typename Address_V>
float3 Linear_2D<Address_U, Address_V>::sample_3(Texture const& texture, float2 uv) const noexcept {
    int4         xy_xy1;
    float2 const st = map(texture, uv, xy_xy1);

    float3 c[4];
    texture.gather_3(xy_xy1, c);

    return bilinear(c, st[0], st[1]);
}

template <typename Address_U, typename Address_V>
float Linear_2D<Address_U, Address_V>::sample_1(Texture const& texture, float2 uv,
                                                int32_t element) const noexcept {
    int4         xy_xy1;
    float2 const st = map(texture, uv, xy_xy1);

    int32_t const min_element = std::min(texture.num_elements() - 1, element);

    float const c00 = texture.at_element_1(xy_xy1[0], xy_xy1[1], min_element);
    float const c01 = texture.at_element_1(xy_xy1[2], xy_xy1[1], min_element);
    float const c10 = texture.at_element_1(xy_xy1[0], xy_xy1[3], min_element);
    float const c11 = texture.at_element_1(xy_xy1[2], xy_xy1[3], min_element);

    return bilinear(c00, c01, c10, c11, st[0], st[1]);
}

template <typename Address_U, typename Address_V>
float2 Linear_2D<Address_U, Address_V>::sample_2(Texture const& texture, float2 uv,
                                                 int32_t element) const noexcept {
    int4         xy_xy1;
    float2 const st = map(texture, uv, xy_xy1);

    int32_t const min_element = std::min(texture.num_elements() - 1, element);

    float2 const c00 = texture.at_element_2(xy_xy1[0], xy_xy1[1], min_element);
    float2 const c01 = texture.at_element_2(xy_xy1[2], xy_xy1[1], min_element);
    float2 const c10 = texture.at_element_2(xy_xy1[0], xy_xy1[3], min_element);
    float2 const c11 = texture.at_element_2(xy_xy1[2], xy_xy1[3], min_element);

    return bilinear(c00, c01, c10, c11, st[0], st[1]);
}

template <typename Address_U, typename Address_V>
float3 Linear_2D<Address_U, Address_V>::sample_3(Texture const& texture, float2 uv,
                                                 int32_t element) const noexcept {
    int4         xy_xy1;
    float2 const st = map(texture, uv, xy_xy1);

    int32_t const min_element = std::min(texture.num_elements() - 1, element);

    float3 const c00 = texture.at_element_3(xy_xy1[0], xy_xy1[1], min_element);
    float3 const c01 = texture.at_element_3(xy_xy1[2], xy_xy1[1], min_element);
    float3 const c10 = texture.at_element_3(xy_xy1[0], xy_xy1[3], min_element);
    float3 const c11 = texture.at_element_3(xy_xy1[2], xy_xy1[3], min_element);

    return bilinear(c00, c01, c10, c11, st[0], st[1]);
}

template <typename Address_U, typename Address_V>
float2 Linear_2D<Address_U, Address_V>::address(float2 uv) const noexcept {
    return float2(Address_U::f(uv[0]), Address_V::f(uv[1]));
}

template <typename Address_U, typename Address_V>
float2 Linear_2D<Address_U, Address_V>::map(Texture const& texture, float2 uv,
                                            int4& xy_xy1) noexcept {
    auto const d = texture.dimensions_float2();

    float const u = Address_U::f(uv[0]) * d[0] - 0.5f;
    float const v = Address_V::f(uv[1]) * d[1] - 0.5f;

    float const fu = std::floor(u);
    float const fv = std::floor(v);

    int32_t const x = static_cast<int32_t>(fu);
    int32_t const y = static_cast<int32_t>(fv);

    auto const b = texture.back_2();

    xy_xy1[0] = Address_U::lower_bound(x, b[0]);
    xy_xy1[1] = Address_V::lower_bound(y, b[1]);
    xy_xy1[2] = Address_U::increment(x, b[0]);
    xy_xy1[3] = Address_V::increment(y, b[1]);

    return float2(u - fu, v - fv);
}

}  // namespace image::texture::sampler

#endif
