#pragma once

#include "sampler_nearest_3d.hpp"
#include "image/texture/texture.hpp"
#include <algorithm>

namespace image { namespace texture { namespace sampler {

template<typename Address_mode>
float Nearest_3D<Address_mode>::sample_1(const Texture& texture, float3_p uvw) const {
	int3 xyz = map(texture, uvw);

	return texture.at_1(xyz.x, xyz.y, xyz.z);
}

template<typename Address_mode>
float2 Nearest_3D<Address_mode>::sample_2(const Texture& texture, float3_p uvw) const {
	int3 xyz = map(texture, uvw);

	return texture.at_2(xyz.x, xyz.y, xyz.z);
}

template<typename Address_mode>
float3 Nearest_3D<Address_mode>::sample_3(const Texture& texture, float3_p uvw) const {
	int3 xyz = map(texture, uvw);

	return texture.at_3(xyz.x, xyz.y, xyz.z);
}

template<typename Address_mode>
float3 Nearest_3D<Address_mode>::address(float3_p uvw) const {
	return Address_mode::f(uvw);
}

template<typename Address_mode>
int3 Nearest_3D<Address_mode>::map(const Texture& texture, float3_p uvw) {
	auto b = texture.back_3();
	auto d = texture.dimensions_float3();

	float3 muvw = Address_mode::f(uvw);

	return int3(std::min(static_cast<int32_t>(muvw.x * d.x), b.x),
				std::min(static_cast<int32_t>(muvw.y * d.y), b.y),
				std::min(static_cast<int32_t>(muvw.z * d.z), b.z));
}

}}}
