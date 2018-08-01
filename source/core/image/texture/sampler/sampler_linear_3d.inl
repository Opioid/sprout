#ifndef SU_CORE_IMAGE_TEXTURE_SAMPLER_LINEAR_3D_INL
#define SU_CORE_IMAGE_TEXTURE_SAMPLER_LINEAR_3D_INL

#include <algorithm>
#include "address_mode.hpp"
#include "bilinear.hpp"
#include "image/texture/texture.hpp"
#include "sampler_linear_3d.hpp"

namespace image::texture::sampler {

template <typename Address_mode>
float Linear_3D<Address_mode>::sample_1(Texture const& texture, f_float3 uvw) const noexcept {
    int3         xyz, xyz1;
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

    return math::lerp(c0, c1, stu[2]);
}

template <typename Address_mode>
float2 Linear_3D<Address_mode>::sample_2(Texture const& texture, f_float3 uvw) const noexcept {
    int3         xyz, xyz1;
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

    return math::lerp(c0, c1, stu[2]);
}

template <typename Address_mode>
float3 Linear_3D<Address_mode>::sample_3(Texture const& texture, f_float3 uvw) const noexcept {
    int3         xyz, xyz1;
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

    return math::lerp(c0, c1, stu[2]);
}

template <typename Address_mode>
float3 Linear_3D<Address_mode>::address(f_float3 uvw) const noexcept {
    return float3(Address_mode::f(uvw[0]), Address_mode::f(uvw[1]), Address_mode::f(uvw[2]));
}

template <typename Address_mode>
float3 Linear_3D<Address_mode>::map(Texture const& texture, f_float3 uvw, int3& xyz,
                                    int3& xyz1) noexcept {
    auto const& b = texture.back_3();
    auto const& d = texture.dimensions_float3();

    float const u = Address_mode::f(uvw[0]) * d[0] - 0.5f;
    float const v = Address_mode::f(uvw[1]) * d[1] - 0.5f;
    float const w = Address_mode::f(uvw[2]) * d[2] - 0.5f;

    float const fu = std::floor(u);
    float const fv = std::floor(v);
    float const fw = std::floor(w);

    int32_t const x = static_cast<int32_t>(fu);
    int32_t const y = static_cast<int32_t>(fv);
    int32_t const z = static_cast<int32_t>(fw);

    xyz[0] = Address_mode::lower_bound(x, b[0]);
    xyz[1] = Address_mode::lower_bound(y, b[1]);
    xyz[2] = Address_mode::lower_bound(z, b[2]);

    xyz1[0] = Address_mode::increment(x, b[0]);
    xyz1[1] = Address_mode::increment(y, b[1]);
    xyz1[2] = Address_mode::increment(z, b[2]);

    return float3(u - fu, v - fv, w - fw);
}

}  // namespace image::texture::sampler

#endif
