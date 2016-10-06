#pragma once

#include "sampler_3d_nearest.hpp"
#include "image/texture/texture.hpp"
#include <algorithm>

namespace image { namespace texture { namespace sampler {

template<typename Address_mode>
float Sampler_3D_nearest<Address_mode>::sample_1(const Texture& texture, float3_p uvw) const {
	auto b = texture.back_3();
	auto d = texture.dimensions_float3();

	float3 xyz = Address_mode::f(uvw);

	int32_t x = std::min(static_cast<int32_t>(xyz.x * d.x), b.x);
	int32_t y = std::min(static_cast<int32_t>(xyz.y * d.y), b.y);
	int32_t z = std::min(static_cast<int32_t>(xyz.z * d.z), b.z);

	return texture.at_1(x, y, z);
}

template<typename Address_mode>
float2 Sampler_3D_nearest<Address_mode>::sample_2(const Texture& texture, float3_p uvw) const {
	auto b = texture.back_3();
	auto d = texture.dimensions_float3();

	float3 xyz = Address_mode::f(uvw);

	int32_t x = std::min(static_cast<int32_t>(xyz.x * d.x), b.x);
	int32_t y = std::min(static_cast<int32_t>(xyz.y * d.y), b.y);
	int32_t z = std::min(static_cast<int32_t>(xyz.z * d.z), b.z);

	return texture.at_2(x, y, z);
}

template<typename Address_mode>
float3 Sampler_3D_nearest<Address_mode>::sample_3(const Texture& texture, float3_p uvw) const {
	auto b = texture.back_3();
	auto d = texture.dimensions_float3();

	float3 xyz = Address_mode::f(uvw);

	int32_t x = std::min(static_cast<int32_t>(xyz.x * d.x), b.x);
	int32_t y = std::min(static_cast<int32_t>(xyz.y * d.y), b.y);
	int32_t z = std::min(static_cast<int32_t>(xyz.z * d.z), b.z);

	return texture.at_3(x, y, z);
}

template<typename Address_mode>
float3 Sampler_3D_nearest<Address_mode>::address(float3_p uvw) const {
	return Address_mode::f(uvw);
}

}}}
