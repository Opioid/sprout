#ifndef SU_CORE_IMAGE_TEXTURE_SAMPLER_LINEAR_3D_INL
#define SU_CORE_IMAGE_TEXTURE_SAMPLER_LINEAR_3D_INL

#include "sampler_linear_3d.hpp"
#include "address_mode.hpp"
#include "bilinear.hpp"
#include "image/texture/texture.hpp"
#include <algorithm>

namespace image::texture::sampler {

template<typename Address_mode>
float Linear_3D<Address_mode>::sample_1(const Texture& texture, const float3& uvw) const {
	int3 xyz, xyz1;
	const float3 stu = map(texture, uvw, xyz, xyz1);

	const float c000 = texture.at_1(xyz[0],  xyz[1],  xyz[2]);
	const float c010 = texture.at_1(xyz1[0], xyz[1],  xyz[2]);
	const float c100 = texture.at_1(xyz[0],  xyz1[1], xyz[2]);
	const float c110 = texture.at_1(xyz1[0], xyz1[1], xyz[2]);
	const float c001 = texture.at_1(xyz[0],  xyz[1],  xyz1[2]);
	const float c011 = texture.at_1(xyz1[0], xyz[1],  xyz1[2]);
	const float c101 = texture.at_1(xyz[0],  xyz1[1], xyz1[2]);
	const float c111 = texture.at_1(xyz1[0], xyz1[1], xyz1[2]);

	const float c0 = bilinear(c000, c010, c100, c110, stu[0], stu[1]);
	const float c1 = bilinear(c001, c011, c101, c111, stu[0], stu[1]);

	return math::lerp(c0, c1, stu[2]);
}

template<typename Address_mode>
float2 Linear_3D<Address_mode>::sample_2(const Texture& texture, const float3& uvw) const {
	int3 xyz, xyz1;
	const float3 stu = map(texture, uvw, xyz, xyz1);

	const float2 c000 = texture.at_2(xyz[0],  xyz[1],  xyz[2]);
	const float2 c010 = texture.at_2(xyz1[0], xyz[1],  xyz[2]);
	const float2 c100 = texture.at_2(xyz[0],  xyz1[1], xyz[2]);
	const float2 c110 = texture.at_2(xyz1[0], xyz1[1], xyz[2]);
	const float2 c001 = texture.at_2(xyz[0],  xyz[1],  xyz1[2]);
	const float2 c011 = texture.at_2(xyz1[0], xyz[1],  xyz1[2]);
	const float2 c101 = texture.at_2(xyz[0],  xyz1[1], xyz1[2]);
	const float2 c111 = texture.at_2(xyz1[0], xyz1[1], xyz1[2]);

	const float2 c0 = bilinear(c000, c010, c100, c110, stu[0], stu[1]);
	const float2 c1 = bilinear(c001, c011, c101, c111, stu[0], stu[1]);

	return math::lerp(c0, c1, stu[2]);
}

template<typename Address_mode>
float3 Linear_3D<Address_mode>::sample_3(const Texture& texture, const float3& uvw) const {
	int3 xyz, xyz1;
	const float3 stu = map(texture, uvw, xyz, xyz1);

	const float3 c000 = texture.at_3(xyz[0],  xyz[1],  xyz[2]);
	const float3 c010 = texture.at_3(xyz1[0], xyz[1],  xyz[2]);
	const float3 c100 = texture.at_3(xyz[0],  xyz1[1], xyz[2]);
	const float3 c110 = texture.at_3(xyz1[0], xyz1[1], xyz[2]);
	const float3 c001 = texture.at_3(xyz[0],  xyz[1],  xyz1[2]);
	const float3 c011 = texture.at_3(xyz1[0], xyz[1],  xyz1[2]);
	const float3 c101 = texture.at_3(xyz[0],  xyz1[1], xyz1[2]);
	const float3 c111 = texture.at_3(xyz1[0], xyz1[1], xyz1[2]);

	const float3 c0 = bilinear(c000, c010, c100, c110, stu[0], stu[1]);
	const float3 c1 = bilinear(c001, c011, c101, c111, stu[0], stu[1]);

	return math::lerp(c0, c1, stu[2]);
}

template<typename Address_mode>
float3 Linear_3D<Address_mode>::address(const float3& uvw) const {
	return float3(Address_mode::f(uvw[0]),
				  Address_mode::f(uvw[1]),
				  Address_mode::f(uvw[2]));
}

template<typename Address_mode>
float3 Linear_3D<Address_mode>::map(const Texture& texture, const float3& uvw,
									int3& xyz, int3& xyz1) {
	const auto& b = texture.back_3();
	const auto& d = texture.dimensions_float3();

	const float u = Address_mode::f(uvw[0]) * d[0] - 0.5f;
	const float v = Address_mode::f(uvw[1]) * d[1] - 0.5f;
	const float w = Address_mode::f(uvw[2]) * d[2] - 0.5f;

	const float fu = std::floor(u);
	const float fv = std::floor(v);
	const float fw = std::floor(w);

	const int32_t x = static_cast<int32_t>(fu);
	const int32_t y = static_cast<int32_t>(fv);
	const int32_t z = static_cast<int32_t>(fw);

	xyz[0] = Address_mode::lower_bound(x, b[0]);
	xyz[1] = Address_mode::lower_bound(y, b[1]);
	xyz[2] = Address_mode::lower_bound(z, b[2]);

	xyz1[0] = Address_mode::increment(x, b[0]);
	xyz1[1] = Address_mode::increment(y, b[1]);
	xyz1[2] = Address_mode::increment(z, b[2]);

	return float3(u - fu, v - fv, w - fw);
}

}

#endif
