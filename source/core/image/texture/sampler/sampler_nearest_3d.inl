#pragma once

#include "sampler_nearest_3d.hpp"
#include "address_mode.hpp"
#include "image/texture/texture.hpp"
#include "math/vector3.inl"
#include <algorithm>

namespace image::texture::sampler {

template<typename Address_mode>
float Nearest_3D<Address_mode>::sample_1(Texture const& texture, f_float3 uvw) const {
	const int3 xyz = map(texture, uvw);

	return texture.at_1(xyz[0], xyz[1], xyz[2]);
}

template<typename Address_mode>
float2 Nearest_3D<Address_mode>::sample_2(Texture const& texture, f_float3 uvw) const {
	const int3 xyz = map(texture, uvw);

	return texture.at_2(xyz[0], xyz[1], xyz[2]);
}

template<typename Address_mode>
float3 Nearest_3D<Address_mode>::sample_3(Texture const& texture, f_float3 uvw) const {
	const int3 xyz = map(texture, uvw);

	return texture.at_3(xyz[0], xyz[1], xyz[2]);
}

template<typename Address_mode>
float3 Nearest_3D<Address_mode>::address(f_float3 uvw) const {
	return float3(Address_mode::f(uvw[0]),
				  Address_mode::f(uvw[1]),
				  Address_mode::f(uvw[2]));
}

template<typename Address_mode>
int3 Nearest_3D<Address_mode>::map(Texture const& texture, f_float3 uvw) {
	auto const& b = texture.back_3();
	auto const& d = texture.dimensions_float3();

	float const u = Address_mode::f(uvw[0]);
	float const v = Address_mode::f(uvw[1]);
	float const w = Address_mode::f(uvw[2]);

	return int3(std::min(static_cast<int32_t>(u * d[0]), b[0]),
				std::min(static_cast<int32_t>(v * d[1]), b[1]),
				std::min(static_cast<int32_t>(w * d[2]), b[2]));
}

}
