#include "texture_sampler.hpp"
#include "address_mode.hpp"
#include "base/math/vector2.inl"
#include "bilinear.hpp"
#include "image/texture/texture.inl"

#include <algorithm>

namespace image::texture {

Sampler_2D::~Sampler_2D() = default;

template <typename Address_U, typename Address_V>
float Nearest_2D<Address_U, Address_V>::sample_1(Texture const& texture,
                                                           float2         uv) const {
    int2 const xy = map(texture, uv);

    return texture.at_1(xy[0], xy[1]);
}

template <typename Address_U, typename Address_V>
float2 Nearest_2D<Address_U, Address_V>::sample_2(Texture const& texture,
                                                            float2         uv) const {
    int2 const xy = map(texture, uv);

    return texture.at_2(xy[0], xy[1]);
}

template <typename Address_U, typename Address_V>
float3 Nearest_2D<Address_U, Address_V>::sample_3(Texture const& texture,
                                                            float2         uv) const {
    int2 const xy = map(texture, uv);

    return texture.at_3(xy[0], xy[1]);
}

template <typename Address_U, typename Address_V>
float Nearest_2D<Address_U, Address_V>::sample_1(Texture const& texture, float2 uv,
                                                           int32_t element) const {
    int2 const xy = map(texture, uv);

    int32_t const min_element = std::min(texture.num_elements() - 1, element);

    return texture.at_element_1(xy[0], xy[1], min_element);
}

template <typename Address_U, typename Address_V>
float2 Nearest_2D<Address_U, Address_V>::sample_2(Texture const& texture, float2 uv,
                                                            int32_t element) const {
    int2 const xy = map(texture, uv);

    int32_t const min_element = std::min(texture.num_elements() - 1, element);

    return texture.at_element_2(xy[0], xy[1], min_element);
}

template <typename Address_U, typename Address_V>
float3 Nearest_2D<Address_U, Address_V>::sample_3(Texture const& texture, float2 uv,
                                                            int32_t element) const {
    int2 const xy = map(texture, uv);

    int32_t const min_element = std::min(texture.num_elements() - 1, element);

    return texture.at_element_3(xy[0], xy[1], min_element);
}

template <typename Address_U, typename Address_V>
float2 Nearest_2D<Address_U, Address_V>::address(float2 uv) const {
    return float2(Address_U::f(uv[0]), Address_V::f(uv[1]));
}

template <typename Address_U, typename Address_V>
int2 Nearest_2D<Address_U, Address_V>::map(Texture const& texture, float2 uv) {
    int2 const d = texture.dimensions().xy();

    float2 const df = float2(d);

    float const u = Address_U::f(uv[0]);
    float const v = Address_V::f(uv[1]);

    int2 const b = d - 1;

    return int2(std::min(int32_t(u * df[0]), b[0]), std::min(int32_t(v * df[1]), b[1]));
}

template <typename Address_U, typename Address_V>
float Linear_2D<Address_U, Address_V>::sample_1(Texture const& texture, float2 uv) const {
    int4         xy_xy1;
    float2 const st = map(texture, uv, xy_xy1);

    float c[4];
    texture.gather_1(xy_xy1, c);

    return bilinear(c, st[0], st[1]);
}

template <typename Address_U, typename Address_V>
float2 Linear_2D<Address_U, Address_V>::sample_2(Texture const& texture, float2 uv) const {
    int4         xy_xy1;
    float2 const st = map(texture, uv, xy_xy1);

    float2 c[4];
    texture.gather_2(xy_xy1, c);

    return bilinear(c, st[0], st[1]);
}

template <typename Address_U, typename Address_V>
float3 Linear_2D<Address_U, Address_V>::sample_3(Texture const& texture, float2 uv) const {
    int4         xy_xy1;
    float2 const st = map(texture, uv, xy_xy1);

    float3 c[4];
    texture.gather_3(xy_xy1, c);

    return bilinear(c, st[0], st[1]);
}

template <typename Address_U, typename Address_V>
float Linear_2D<Address_U, Address_V>::sample_1(Texture const& texture, float2 uv,
                                                int32_t element) const {
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
                                                 int32_t element) const {
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
                                                 int32_t element) const {
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
float2 Linear_2D<Address_U, Address_V>::address(float2 uv) const {
    return float2(Address_U::f(uv[0]), Address_V::f(uv[1]));
}

template <typename Address_U, typename Address_V>
float2 Linear_2D<Address_U, Address_V>::map(Texture const& texture, float2 uv, int4& xy_xy1) {
    int2 const d = texture.dimensions().xy();

    float2 const df = float2(d);

    float const u = Address_U::f(uv[0]) * df[0] - 0.5f;
    float const v = Address_V::f(uv[1]) * df[1] - 0.5f;

    float const fu = std::floor(u);
    float const fv = std::floor(v);

    int32_t const x = int32_t(fu);
    int32_t const y = int32_t(fv);

    int2 const b = d - 1;

    xy_xy1[0] = Address_U::lower_bound(x, b[0]);
    xy_xy1[1] = Address_V::lower_bound(y, b[1]);
    xy_xy1[2] = Address_U::increment(x, b[0]);
    xy_xy1[3] = Address_V::increment(y, b[1]);

    return float2(u - fu, v - fv);
}

Sampler_3D::~Sampler_3D() = default;

template <typename Address_mode>
Stochastic_3D<Address_mode>::~Stochastic_3D() = default;

template <typename Address_mode>
float Stochastic_3D<Address_mode>::stochastic_1(Texture const& texture, float3_p uvw, float3_p r) const {
    int3 const xyz = map(texture, uvw, r);

    return texture.at_1(xyz[0], xyz[1], xyz[2]);
}

template <typename Address_mode>
float2 Stochastic_3D<Address_mode>::stochastic_2(Texture const& texture, float3_p uvw, float3_p r) const {
    int3 const xyz = map(texture, uvw, r);

    return texture.at_2(xyz[0], xyz[1], xyz[2]);
}

template <typename Address_mode>
float3 Stochastic_3D<Address_mode>::stochastic_3(Texture const& texture, float3_p uvw, float3_p r) const {
    int3 const xyz = map(texture, uvw, r);

    return texture.at_3(xyz[0], xyz[1], xyz[2]);
}

template <typename Address_mode>
float4 Stochastic_3D<Address_mode>::stochastic_4(Texture const& texture, float3_p uvw, float3_p r) const {
    int3 const xyz = map(texture, uvw, r);

    return texture.at_4(xyz[0], xyz[1], xyz[2]);
}

template <typename Address_mode>
float3 Stochastic_3D<Address_mode>::address(float3_p uvw) const {
    return float3(Address_mode::f(uvw[0]), Address_mode::f(uvw[1]), Address_mode::f(uvw[2]));
}

template <typename Address_mode>
int3 Stochastic_3D<Address_mode>::map(Texture const& texture, float3_p uvw, float3_p r) {
    int3 const d = texture.dimensions();

    float3 const df = float3(d);

    float const u = Address_mode::f(uvw[0]);
    float const v = Address_mode::f(uvw[1]);
    float const w = Address_mode::f(uvw[2]);

    int3 const b = d - 1;

    return int3(Address_mode::bound(int32_t(u * df[0] + r[0] - 0.5f), b[0]),
                Address_mode::bound(int32_t(v * df[1] + r[1] - 0.5f), b[1]),
                Address_mode::bound(int32_t(w * df[2] + r[2] - 0.5f), b[2]));
}

template <typename Address_mode>
float Nearest_3D<Address_mode>::sample_1(Texture const& texture, float3_p uvw) const {
    int3 const xyz = map(texture, uvw);

    return texture.at_1(xyz[0], xyz[1], xyz[2]);
}

template <typename Address_mode>
float2 Nearest_3D<Address_mode>::sample_2(Texture const& texture, float3_p uvw) const {
    int3 const xyz = map(texture, uvw);

    return texture.at_2(xyz[0], xyz[1], xyz[2]);
}

template <typename Address_mode>
float3 Nearest_3D<Address_mode>::sample_3(Texture const& texture, float3_p uvw) const {
    int3 const xyz = map(texture, uvw);

    return texture.at_3(xyz[0], xyz[1], xyz[2]);
}

template <typename Address_mode>
float4 Nearest_3D<Address_mode>::sample_4(Texture const& texture, float3_p uvw) const {
    int3 const xyz = map(texture, uvw);

    return texture.at_4(xyz[0], xyz[1], xyz[2]);
}

template <typename Address_mode>
int3 Nearest_3D<Address_mode>::map(Texture const& texture, float3_p uvw) {
    int3 const d = texture.dimensions();

    float3 const df = float3(d);

    float const u = Address_mode::f(uvw[0]);
    float const v = Address_mode::f(uvw[1]);
    float const w = Address_mode::f(uvw[2]);

    int3 const b = d - 1;

    return int3(std::min(int32_t(u * df[0]), b[0]), std::min(int32_t(v * df[1]), b[1]),
                std::min(int32_t(w * df[2]), b[2]));
}

template <typename Address_mode>
float Linear_3D<Address_mode>::sample_1(Texture const& texture, float3_p uvw) const {
    int3         xyz;
    int3         xyz1;
    float3 const stu = map(texture, uvw, xyz, xyz1);

    float c[8];
    texture.gather_1(xyz, xyz1, c);

    float const c0 = bilinear(c[0], c[1], c[2], c[3], stu[0], stu[1]);
    float const c1 = bilinear(c[4], c[5], c[6], c[7], stu[0], stu[1]);

    return lerp(c0, c1, stu[2]);
}

template <typename Address_mode>
float2 Linear_3D<Address_mode>::sample_2(Texture const& texture, float3_p uvw) const {
    int3         xyz;
    int3         xyz1;
    float3 const stu = map(texture, uvw, xyz, xyz1);

    float2 c[8];
    texture.gather_2(xyz, xyz1, c);

    float2 const c0 = bilinear(c[0], c[1], c[2], c[3], stu[0], stu[1]);
    float2 const c1 = bilinear(c[4], c[5], c[6], c[7], stu[0], stu[1]);

    return lerp(c0, c1, stu[2]);
}

template <typename Address_mode>
float3 Linear_3D<Address_mode>::sample_3(Texture const& texture, float3_p uvw) const {
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
float4 Linear_3D<Address_mode>::sample_4(Texture const& texture, float3_p uvw) const {
    int3         xyz;
    int3         xyz1;
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
float3 Linear_3D<Address_mode>::map(Texture const& texture, float3_p uvw, int3& xyz0, int3& xyz1) {
    int3 const d = texture.dimensions();

    float3 const df = float3(d);

    float const u = Address_mode::f(uvw[0]) * df[0] - 0.5f;
    float const v = Address_mode::f(uvw[1]) * df[1] - 0.5f;
    float const w = Address_mode::f(uvw[2]) * df[2] - 0.5f;

    float const fu = std::floor(u);
    float const fv = std::floor(v);
    float const fw = std::floor(w);

    int32_t const x = int32_t(fu);
    int32_t const y = int32_t(fv);
    int32_t const z = int32_t(fw);

    int3 const b = d - 1;

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


template class Stochastic_3D<Address_mode_clamp>;
template class Stochastic_3D<Address_mode_repeat>;

template class Nearest_3D<Address_mode_clamp>;
template class Nearest_3D<Address_mode_repeat>;

template class Linear_3D<Address_mode_clamp>;
template class Linear_3D<Address_mode_repeat>;


}  // namespace image::texture
