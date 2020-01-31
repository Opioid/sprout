#include "texture_sampler.hpp"
#include "address_mode.hpp"
#include "base/math/vector2.inl"
#include "bilinear.hpp"
#include "image/texture/texture.inl"

#include <algorithm>

namespace image::texture {

Sampler_2D::~Sampler_2D() noexcept {}

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
    auto const d = texture.dimensions_float2();

    float const u = Address_mode_U::f(uv[0]);
    float const v = Address_mode_V::f(uv[1]);

    auto const b = texture.back_2();

    return int2(std::min(int32_t(u * d[0]), b[0]), std::min(int32_t(v * d[1]), b[1]));
}

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

    int32_t const x = int32_t(fu);
    int32_t const y = int32_t(fv);

    auto const b = texture.back_2();

    xy_xy1[0] = Address_U::lower_bound(x, b[0]);
    xy_xy1[1] = Address_V::lower_bound(y, b[1]);
    xy_xy1[2] = Address_U::increment(x, b[0]);
    xy_xy1[3] = Address_V::increment(y, b[1]);

    return float2(u - fu, v - fv);
}

Sampler_3D::~Sampler_3D() noexcept {}

template <typename Address_mode>
float Nearest_3D<Address_mode>::sample_1(Texture const& texture, float3 const& uvw) const noexcept {
    int3 const xyz = map(texture, uvw);

    return texture.at_1(xyz[0], xyz[1], xyz[2]);
}

template <typename Address_mode>
float2 Nearest_3D<Address_mode>::sample_2(Texture const& texture, float3 const& uvw) const
    noexcept {
    int3 const xyz = map(texture, uvw);

    return texture.at_2(xyz[0], xyz[1], xyz[2]);
}

template <typename Address_mode>
float3 Nearest_3D<Address_mode>::sample_3(Texture const& texture, float3 const& uvw) const
    noexcept {
    int3 const xyz = map(texture, uvw);

    return texture.at_3(xyz[0], xyz[1], xyz[2]);
}

template <typename Address_mode>
float4 Nearest_3D<Address_mode>::sample_4(Texture const& texture, float3 const& uvw) const
    noexcept {
    int3 const xyz = map(texture, uvw);

    return texture.at_4(xyz[0], xyz[1], xyz[2]);
}

template <typename Address_mode>
float3 Nearest_3D<Address_mode>::address(float3 const& uvw) const noexcept {
    return float3(Address_mode::f(uvw[0]), Address_mode::f(uvw[1]), Address_mode::f(uvw[2]));
}

template <typename Address_mode>
int3 Nearest_3D<Address_mode>::map(Texture const& texture, float3 const& uvw) noexcept {
    auto const& d = texture.dimensions_float3();

    float const u = Address_mode::f(uvw[0]);
    float const v = Address_mode::f(uvw[1]);
    float const w = Address_mode::f(uvw[2]);

    auto const& b = texture.back_3();

    return int3(std::min(int32_t(u * d[0]), b[0]), std::min(int32_t(v * d[1]), b[1]),
                std::min(int32_t(w * d[2]), b[2]));
}

template <typename Address_mode>
float Linear_3D<Address_mode>::sample_1(Texture const& texture, float3 const& uvw) const noexcept {
    int3         xyz;
    int3         xyz1;
    float3 const stu = map(texture, uvw, xyz, xyz1);

    float const c000 = texture.at_1(xyz[0], xyz[1], xyz[2]);
    float const c100 = texture.at_1(xyz1[0], xyz[1], xyz[2]);
    float const c010 = texture.at_1(xyz[0], xyz1[1], xyz[2]);
    float const c110 = texture.at_1(xyz1[0], xyz1[1], xyz[2]);
    float const c001 = texture.at_1(xyz[0], xyz[1], xyz1[2]);
    float const c101 = texture.at_1(xyz1[0], xyz[1], xyz1[2]);
    float const c011 = texture.at_1(xyz[0], xyz1[1], xyz1[2]);
    float const c111 = texture.at_1(xyz1[0], xyz1[1], xyz1[2]);

    float const c0 = bilinear(c000, c100, c010, c110, stu[0], stu[1]);
    float const c1 = bilinear(c001, c101, c011, c111, stu[0], stu[1]);

    return lerp(c0, c1, stu[2]);
}

template <typename Address_mode>
float2 Linear_3D<Address_mode>::sample_2(Texture const& texture, float3 const& uvw) const noexcept {
    int3         xyz;
    int3         xyz1;
    float3 const stu = map(texture, uvw, xyz, xyz1);

    float2 const c000 = texture.at_2(xyz[0], xyz[1], xyz[2]);
    float2 const c100 = texture.at_2(xyz1[0], xyz[1], xyz[2]);
    float2 const c010 = texture.at_2(xyz[0], xyz1[1], xyz[2]);
    float2 const c110 = texture.at_2(xyz1[0], xyz1[1], xyz[2]);
    float2 const c001 = texture.at_2(xyz[0], xyz[1], xyz1[2]);
    float2 const c101 = texture.at_2(xyz1[0], xyz[1], xyz1[2]);
    float2 const c011 = texture.at_2(xyz[0], xyz1[1], xyz1[2]);
    float2 const c111 = texture.at_2(xyz1[0], xyz1[1], xyz1[2]);

    float2 const c0 = bilinear(c000, c100, c010, c110, stu[0], stu[1]);
    float2 const c1 = bilinear(c001, c101, c011, c111, stu[0], stu[1]);

    return lerp(c0, c1, stu[2]);
}

template <typename Address_mode>
float3 Linear_3D<Address_mode>::sample_3(Texture const& texture, float3 const& uvw) const noexcept {
    int3         xyz;
    int3         xyz1;
    float3 const stu = map(texture, uvw, xyz, xyz1);

    float3 const c000 = texture.at_3(xyz[0], xyz[1], xyz[2]);
    float3 const c100 = texture.at_3(xyz1[0], xyz[1], xyz[2]);
    float3 const c010 = texture.at_3(xyz[0], xyz1[1], xyz[2]);
    float3 const c110 = texture.at_3(xyz1[0], xyz1[1], xyz[2]);
    float3 const c001 = texture.at_3(xyz[0], xyz[1], xyz1[2]);
    float3 const c101 = texture.at_3(xyz1[0], xyz[1], xyz1[2]);
    float3 const c011 = texture.at_3(xyz[0], xyz1[1], xyz1[2]);
    float3 const c111 = texture.at_3(xyz1[0], xyz1[1], xyz1[2]);

    float3 const c0 = bilinear(c000, c100, c010, c110, stu[0], stu[1]);
    float3 const c1 = bilinear(c001, c101, c011, c111, stu[0], stu[1]);

    return lerp(c0, c1, stu[2]);
}

template <typename Address_mode>
float4 Linear_3D<Address_mode>::sample_4(Texture const& texture, float3 const& uvw) const noexcept {
    int3         xyz, xyz1;
    float3 const stu = map(texture, uvw, xyz, xyz1);

    float4 const c000 = texture.at_4(xyz[0], xyz[1], xyz[2]);
    float4 const c100 = texture.at_4(xyz1[0], xyz[1], xyz[2]);
    float4 const c010 = texture.at_4(xyz[0], xyz1[1], xyz[2]);
    float4 const c110 = texture.at_4(xyz1[0], xyz1[1], xyz[2]);
    float4 const c001 = texture.at_4(xyz[0], xyz[1], xyz1[2]);
    float4 const c101 = texture.at_4(xyz1[0], xyz[1], xyz1[2]);
    float4 const c011 = texture.at_4(xyz[0], xyz1[1], xyz1[2]);
    float4 const c111 = texture.at_4(xyz1[0], xyz1[1], xyz1[2]);

    float4 const c0 = bilinear(c000, c100, c010, c110, stu[0], stu[1]);
    float4 const c1 = bilinear(c001, c101, c011, c111, stu[0], stu[1]);

    return lerp(c0, c1, stu[2]);
}

template <typename Address_mode>
float3 Linear_3D<Address_mode>::address(float3 const& uvw) const noexcept {
    return float3(Address_mode::f(uvw[0]), Address_mode::f(uvw[1]), Address_mode::f(uvw[2]));
}

template <typename Address_mode>
float3 Linear_3D<Address_mode>::map(Texture const& texture, float3 const& uvw, int3& xyz0,
                                    int3& xyz1) noexcept {
    auto const& d = texture.dimensions_float3();

    float const u = Address_mode::f(uvw[0]) * d[0] - 0.5f;
    float const v = Address_mode::f(uvw[1]) * d[1] - 0.5f;
    float const w = Address_mode::f(uvw[2]) * d[2] - 0.5f;

    float const fu = std::floor(u);
    float const fv = std::floor(v);
    float const fw = std::floor(w);

    int32_t const x = int32_t(fu);
    int32_t const y = int32_t(fv);
    int32_t const z = int32_t(fw);

    auto const& b = texture.back_3();

    xyz0[0] = Address_mode::lower_bound(x, b[0]);
    xyz0[1] = Address_mode::lower_bound(y, b[1]);
    xyz0[2] = Address_mode::lower_bound(z, b[2]);

    xyz1[0] = Address_mode::increment(x, b[0]);
    xyz1[1] = Address_mode::increment(y, b[1]);
    xyz1[2] = Address_mode::increment(z, b[2]);

    return float3(u - fu, v - fv, w - fw);
}

template class Nearest_2D<Address_mode_clamp, Address_mode_clamp>;
template class Nearest_2D<Address_mode_clamp, Address_mode_repeat>;
template class Nearest_2D<Address_mode_repeat, Address_mode_clamp>;
template class Nearest_2D<Address_mode_repeat, Address_mode_repeat>;

template class Linear_2D<Address_mode_clamp, Address_mode_clamp>;
template class Linear_2D<Address_mode_clamp, Address_mode_repeat>;
template class Linear_2D<Address_mode_repeat, Address_mode_clamp>;
template class Linear_2D<Address_mode_repeat, Address_mode_repeat>;

template class Nearest_3D<Address_mode_clamp>;
template class Nearest_3D<Address_mode_repeat>;

template class Linear_3D<Address_mode_clamp>;
template class Linear_3D<Address_mode_repeat>;

}  // namespace image::texture
