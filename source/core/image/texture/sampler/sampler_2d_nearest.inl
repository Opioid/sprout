#pragma once

#include "sampler_2d_nearest.hpp"
#include "image/texture/texture.hpp"
#include <algorithm>

namespace image { namespace texture { namespace sampler {

template<typename Address_mode>
float Sampler_2D_nearest<Address_mode>::sample_1(const Texture& texture, float2 uv) const {
	int2 xy = map(texture, uv);

	return texture.at_1(xy.x, xy.y);
}

template<typename Address_mode>
float2 Sampler_2D_nearest<Address_mode>::sample_2(const Texture& texture, float2 uv) const {
	int2 xy = map(texture, uv);

	return texture.at_2(xy.x, xy.y);
}

template<typename Address_mode>
float3 Sampler_2D_nearest<Address_mode>::sample_3(const Texture& texture, float2 uv) const {
	int2 xy = map(texture, uv);

	return texture.at_3(xy.x, xy.y);
}

template<typename Address_mode>
float Sampler_2D_nearest<Address_mode>::sample_1(const Texture& texture, float2 uv,
												 int32_t element) const {
	int2 xy = map(texture, uv);

	int32_t min_element = std::min(texture.num_elements() - 1, element);

	return texture.at_element_1(xy.x, xy.y, min_element);
}

template<typename Address_mode>
float2 Sampler_2D_nearest<Address_mode>::sample_2(const Texture& texture, float2 uv,
												  int32_t element) const {
	int2 xy = map(texture, uv);

	int32_t min_element = std::min(texture.num_elements() - 1, element);

	return texture.at_element_2(xy.x, xy.y, min_element);
}

template<typename Address_mode>
float3 Sampler_2D_nearest<Address_mode>::sample_3(const Texture& texture, float2 uv,
												  int32_t element) const {
	int2 xy = map(texture, uv);

	int32_t min_element = std::min(texture.num_elements() - 1, element);

	return texture.at_element_3(xy.x, xy.y, min_element);
}

template<typename Address_mode>
float2 Sampler_2D_nearest<Address_mode>::address(float2 uv) const {
	return Address_mode::f(uv);
}

template<typename Address_mode>
int2 Sampler_2D_nearest<Address_mode>::map(const Texture& texture, float2 uv) {
	auto b = texture.back_2();
	auto d = texture.dimensions_float2();

	uv = Address_mode::f(uv);

	return int2(std::min(static_cast<int32_t>(uv.x * d.x), b.x),
				std::min(static_cast<int32_t>(uv.y * d.y), b.y));
}

}}}
