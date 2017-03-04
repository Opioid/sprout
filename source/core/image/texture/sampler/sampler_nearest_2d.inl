#pragma once

#include "sampler_nearest_2d.hpp"
#include "address_mode.hpp"
#include "image/texture/texture.hpp"
#include <algorithm>

namespace image { namespace texture { namespace sampler {

template<typename Address_mode_U, typename Address_mode_V>
float Nearest_2D<Address_mode_U, Address_mode_V>::sample_1(const Texture& texture,
														   float2 uv) const {
	int2 xy = map(texture, uv);

	return texture.at_1(xy.v[0], xy.v[1]);
}

template<typename Address_mode_U, typename Address_mode_V>
float2 Nearest_2D<Address_mode_U, Address_mode_V>::sample_2(const Texture& texture,
															float2 uv) const {
	int2 xy = map(texture, uv);

	return texture.at_2(xy.v[0], xy.v[1]);
}

template<typename Address_mode_U, typename Address_mode_V>
float3 Nearest_2D<Address_mode_U, Address_mode_V>::sample_3(const Texture& texture,
															float2 uv) const {
	int2 xy = map(texture, uv);

	return texture.at_3(xy.v[0], xy.v[1]);
}

template<typename Address_mode_U, typename Address_mode_V>
float Nearest_2D<Address_mode_U, Address_mode_V>::sample_1(const Texture& texture, float2 uv,
														   int32_t element) const {
	int2 xy = map(texture, uv);

	int32_t min_element = std::min(texture.num_elements() - 1, element);

	return texture.at_element_1(xy.v[0], xy.v[1], min_element);
}

template<typename Address_mode_U, typename Address_mode_V>
float2 Nearest_2D<Address_mode_U, Address_mode_V>::sample_2(const Texture& texture, float2 uv,
															int32_t element) const {
	int2 xy = map(texture, uv);

	int32_t min_element = std::min(texture.num_elements() - 1, element);

	return texture.at_element_2(xy.v[0], xy.v[1], min_element);
}

template<typename Address_mode_U, typename Address_mode_V>
float3 Nearest_2D<Address_mode_U, Address_mode_V>::sample_3(const Texture& texture, float2 uv,
															int32_t element) const {
	int2 xy = map(texture, uv);

	int32_t min_element = std::min(texture.num_elements() - 1, element);

	return texture.at_element_3(xy.v[0], xy.v[1], min_element);
}

template<typename Address_mode_U, typename Address_mode_V>
float2 Nearest_2D<Address_mode_U, Address_mode_V>::address(float2 uv) const {
	return float2(Address_mode_U::f(uv.v[0]), Address_mode_V::f(uv.v[1]));
}

template<typename Address_mode_U, typename Address_mode_V>
int2 Nearest_2D<Address_mode_U, Address_mode_V>::map(const Texture& texture, float2 uv) {
	auto b = texture.back_2();
	const auto d = texture.dimensions_float2();

	float u = Address_mode_U::f(uv.v[0]);
	float v = Address_mode_V::f(uv.v[1]);

	return int2(std::min(static_cast<int32_t>(u * d.v[0]), b.v[0]),
				std::min(static_cast<int32_t>(v * d.v[1]), b.v[1]));
}

}}}
